#include "mili/decision_engine.hpp"
#include "mili/log_exporter.hpp"
#include "mili/log_filter.hpp"
#include "mili/mock_products.hpp"
#include "mili/ring_buffer_logger.hpp"
#include "mili/uart_log_exporter.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;
std::vector<std::string> uart_lines;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

bool uart_capture(const char* data, std::size_t len, void*) {
    uart_lines.emplace_back(data, len);
    return true;
}

void test_file_export_csv_json() {
    mili::DecisionLogger logger(5);
    mili::SensorInput input{};
    input.battery_level = 72.0f;
    input.threat_level = 0.3f;
    mili::DecisionResult result{};
    result.selected_mode = mili::OperationalMode::Surveillance;
    result.selected_score = 0.81f;
    result.confidence = 0.72f;
    result.low_confidence = false;
    result.degraded_inputs = true;
    logger.log(input, result, 1000);

    const auto csv = mili::LogExporter::to_csv(logger);
    const auto json = mili::LogExporter::to_json(logger);
    check(csv.find("surveillance") != std::string::npos, "CSV contains mode");
    check(csv.find("degraded_inputs") != std::string::npos, "CSV header has degraded_inputs");
    check(json.find("\"version\":1") != std::string::npos, "JSON has version");

    const std::string csv_path = "test_export.csv";
    const std::string json_path = "test_export.json";
    check(mili::LogExporter::write_csv(logger, csv_path), "Write CSV file");
    check(mili::LogExporter::write_json(logger, json_path), "Write JSON file");
}

void test_embedded_ring_buffer_export() {
    mili::EmbeddedDecisionLogger logger;
    for (int i = 0; i < 5; ++i) {
        mili::SensorInput input{};
        input.battery_level = 60.0f + static_cast<float>(i);
        mili::DecisionResult result{};
        result.selected_mode = mili::OperationalMode::Reconnaissance;
        result.selected_score = 0.5f + static_cast<float>(i) * 0.05f;
        result.confidence = 0.6f;
        logger.log(input, result, static_cast<std::uint64_t>(i * 100));
    }

    check(logger.capacity() == mili::kEmbeddedLogCapacity, "Embedded ring capacity");
    check(logger.size() == 5, "Ring buffer stores entries");

    const auto csv = mili::LogExporter::to_csv(logger);
    check(csv.find("reconnaissance") != std::string::npos, "Ring buffer export via IDecisionLogger");

    uart_lines.clear();
    mili::UartLogExporter uart(uart_capture, nullptr);
    check(uart.write_header(), "UART header");
    const auto sent = uart.drain(logger);
    check(sent == 5, "UART drain sends all lines");
    check(uart_lines.size() >= 6, "UART header + data lines");
}

void test_log_filter_search() {
    mili::DecisionLogger logger(20);
    for (int i = 0; i < 10; ++i) {
        mili::SensorInput input{};
        mili::DecisionResult result{};
        result.selected_mode = (i % 2 == 0) ? mili::OperationalMode::Loiter : mili::OperationalMode::Engagement;
        result.confidence = static_cast<float>(i) / 10.0f;
        result.low_confidence = result.confidence < 0.5f;
        result.mode_changed = i % 3 == 0;
        logger.log(input, result, static_cast<std::uint64_t>(i * 1000));
    }

    mili::LogQuery query{};
    query.mode_filter = mili::OperationalMode::Loiter;
    check(mili::LogFilter::count(logger, query) == 5, "Filter by mode");

    query = {};
    query.low_confidence_only = true;
    check(mili::LogFilter::count(logger, query) == 5, "Filter low confidence");

    query = {};
    query.mode_changed_only = true;
    const auto matches = mili::LogFilter::apply(logger, query);
    check(matches.size() == 4, "Filter mode changes");
}

}  // namespace

int main() {
    test_file_export_csv_json();
    test_embedded_ring_buffer_export();
    test_log_filter_search();

    if (failures > 0) {
        std::cerr << failures << " reporting test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All reporting tests passed.\n";
    return EXIT_SUCCESS;
}

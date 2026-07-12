#include "mili/data_quality_monitor.hpp"
#include "mili/embedded_config.hpp"
#include "mili/integration/integration_hub.hpp"
#include "mili/log_exporter.hpp"
#include "mili/mock_products.hpp"
#include "mili/protocol/messages.hpp"
#include "mili/ring_buffer_logger.hpp"
#include "mili/topsis_scorer.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_can_protocol_roundtrip() {
    mili::protocol::Product1CanPayload p1{};
    p1.battery_level_x100 = 7250;
    p1.power_draw_x10 = 120;
    p1.flags = 0x01;

    const auto decoded = mili::protocol::decode_product1(p1, 1000);
    check(decoded.valid, "Product1 CAN payload should decode");
    check(std::fabs(decoded.battery_level_percent - 72.5f) < 0.01f, "Battery decode accuracy");

    mili::DecisionResult result{};
    result.selected_mode = mili::OperationalMode::Engagement;
    result.control_parameters.speed_factor = 0.9f;
    const auto encoded = mili::protocol::encode_mode_command(result);
    check(encoded.mode == static_cast<std::uint8_t>(mili::OperationalMode::Engagement), "Mode encode");
    check(encoded.speed_factor_x100 == 90, "Speed factor encode");
}

void test_integration_hub_can_clients() {
    mili::SensorAdapter adapter;
    std::vector<std::pair<std::uint16_t, std::size_t>> sent;
    mili::CanControlPublisher publisher([&](std::uint16_t id, const void*, std::size_t len) {
        sent.emplace_back(id, len);
        return true;
    });

    mili::IntegrationHub hub(adapter, publisher);
    mili::CanProduct1Client p1_client;
    mili::CanProduct2Client p2_client;
    hub.set_product1_client(&p1_client);
    hub.set_product2_client(&p2_client);

    mili::protocol::Product1CanPayload p1_frame{};
    p1_frame.battery_level_x100 = 8000;
    p1_client.on_frame(p1_frame, 1000);

    mili::protocol::Product2CanPayload p2_frame{};
    p2_frame.distance_home_x10 = 50;
    p2_client.on_frame(p2_frame, 1000);

    adapter.ingest_environmental(mili::mock_environmental(1000), 1000);

    mili::DecisionEngine engine(0);
    const auto result = hub.run_cycle(engine, 1000);

    check(hub.product1_connected(), "Product1 client should be connected");
    check(hub.product2_connected(), "Product2 client should be connected");
    check(!sent.empty(), "Control publisher should emit CAN frame");
    check(sent[0].first == mili::protocol::kCanIdModeCommand, "Mode command CAN ID");
    check(result.selected_score > 0.0f, "Integration cycle should produce decision");
}

void test_data_quality_emergency_return() {
    mili::DataQualityMonitor monitor(500);
    mili::Product1Data p1{};
    mili::Product2Data p2{};
    mili::EnvironmentalData env{};
    mili::OperatorCommand op{};

    const auto status = monitor.evaluate(p1, p2, env, op, 1000);
    check(status.emergency_return, "Missing critical inputs should trigger emergency return");
    check(monitor.should_force_return_home(status, 50.0f), "Emergency status should force return home");
    check(monitor.should_force_return_home(status, 10.0f), "Low battery should force return home");
}

void test_log_exporter() {
    mili::DecisionLogger logger(10);
    mili::SensorInput input{};
    mili::DecisionResult result{};
    result.selected_mode = mili::OperationalMode::Surveillance;
    result.selected_score = 0.8f;
    result.confidence = 0.7f;
    logger.log(input, result, 42);

    const auto csv = mili::LogExporter::to_csv(logger);
    const auto json = mili::LogExporter::to_json(logger);
    check(csv.find("surveillance") != std::string::npos, "CSV export should include mode");
    check(json.find("\"timestamp_ms\":42") != std::string::npos, "JSON export should include timestamp");
}

void test_topsis_scorer() {
    mili::McdmEngine scorer;
    mili::SensorInput input{};
    input.battery_level = 80.0f;
    input.threat_level = 0.2f;
    input.target_visibility = 0.9f;

    const auto scores = scorer.compute_scores(input);
    const auto& topsis = scorer.last_topsis();
    check(topsis.closeness[0] >= 0.0f && topsis.closeness[0] <= 1.0f, "TOPSIS closeness bounded");
    check(scores[0] != scores[1], "MCDM hybrid scores should differentiate modes");
}

void test_ring_buffer_logger() {
    mili::EmbeddedDecisionLogger logger;
    mili::SensorInput input{};
    mili::DecisionResult result{};
    for (int i = 0; i < 1005; ++i) {
        logger.log(input, result, static_cast<std::uint64_t>(i));
    }
    check(logger.size() == mili::kEmbeddedLogCapacity, "Ring buffer should cap at max entries");
    check(logger.at(0).timestamp_ms == 973, "Ring buffer should keep most recent entries");
}

}  // namespace

int main() {
    test_can_protocol_roundtrip();
    test_integration_hub_can_clients();
    test_data_quality_emergency_return();
    test_log_exporter();
    test_topsis_scorer();
    test_ring_buffer_logger();

    if (failures > 0) {
        std::cerr << failures << " evolution test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All evolution tests passed.\n";
    return EXIT_SUCCESS;
}

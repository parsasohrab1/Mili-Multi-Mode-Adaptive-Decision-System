#include "mili/decision_engine.hpp"
#include "mili/embedded_config.hpp"
#include "mili/integration/integration_hub.hpp"
#include "mili/platform/benchmark.hpp"
#include "mili/platform/can_router.hpp"
#include "mili/protocol/messages.hpp"
#include "mili/ring_buffer_logger.hpp"

#include "mili_app.hpp"

#include "can_hal.hpp"
#include "spi_hal.hpp"
#include "uart_hal.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_embedded_logger_no_heap() {
    mili::EmbeddedDecisionLogger logger;
    mili::SensorInput input{};
    mili::DecisionResult result{};
    result.selected_mode = mili::OperationalMode::Surveillance;
    logger.log(input, result, 1);

    check(logger.size() == 1, "Embedded logger should store entry");
    check(logger.capacity() == mili::kEmbeddedLogCapacity, "Embedded log capacity should be 32");
    check(mili::estimated_log_ram_bytes(logger.capacity()) < mili::kEmbeddedRamBudgetBytes,
        "Log RAM should stay within 64 KB budget");

    mili::DecisionLogEntry entry{};
    check(logger.entry_at(0, entry), "entry_at should work via IDecisionLogger");
}

void test_decision_report_protocol() {
    mili::DecisionResult result{};
    result.selected_mode = mili::OperationalMode::Pursuit;
    result.confidence = 0.82f;
    result.selected_score = 0.67f;
    result.mode_changed = true;

    const auto report = mili::protocol::encode_decision_report(result, 1200);
    check(report.mode == static_cast<std::uint8_t>(mili::OperationalMode::Pursuit), "Report mode");
    check(report.confidence_x100 == 82, "Report confidence");
    check(report.evaluate_time_us == 1200, "Report timing");
    check(report.protocol_version == mili::protocol::kProtocolVersion, "Report protocol version");
}

void test_config_update_protocol() {
    const auto payload = mili::protocol::encode_config_update(
        mili::protocol::ConfigCommand::SetHysteresis,
        mili::OperationalMode::Reconnaissance,
        0,
        0.5f);

    mili::protocol::ConfigCommand command{};
    mili::OperationalMode mode{};
    std::uint8_t factor_id = 0;
    float value = 0.0f;
    check(mili::protocol::decode_config_update(payload, command, mode, factor_id, value), "Config decode");
    check(command == mili::protocol::ConfigCommand::SetHysteresis, "Config command");
    check(value == 0.5f, "Config value");
}

bool test_send_frame(std::uint16_t can_id, const void* data, std::size_t len, void* user_data) {
    auto* frames = static_cast<std::vector<std::pair<std::uint16_t, std::size_t>>*>(user_data);
    frames->emplace_back(can_id, len);
    return true;
}

void test_can_router_and_report() {
    mili::SensorAdapter adapter;
    mili::CanProduct1Client p1;
    mili::CanProduct2Client p2;
    mili::CanOperatorClient op;
    mili::CanRouter router(p1, p2, op, adapter);

    mili::protocol::Product1CanPayload p1_frame{};
    p1_frame.battery_level_x100 = 6500;
    router.route_frame(
        mili::protocol::kCanIdProduct1Status,
        reinterpret_cast<const std::uint8_t*>(&p1_frame),
        sizeof(p1_frame),
        100);

  mili::protocol::OperatorCanPayload op_frame{};
    op_frame.flags = 0x01;
    op_frame.priority_override = static_cast<std::uint8_t>(mili::MissionPriority::Critical);
    router.route_frame(
        mili::protocol::kCanIdOperatorCmd,
        reinterpret_cast<const std::uint8_t*>(&op_frame),
        sizeof(op_frame),
        100);

    mili::OperatorCommand command{};
    check(op.poll(command, 150), "Operator CAN frame should be routable");

    std::vector<std::pair<std::uint16_t, std::size_t>> sent;
    mili::CanReportPublisher reporter(test_send_frame, &sent);
    mili::DecisionResult result{};
    result.selected_mode = mili::OperationalMode::Engagement;
    result.confidence = 0.9f;
    result.selected_score = 0.88f;
    check(reporter.publish(result, 500), "Decision report publish");
    check(!sent.empty(), "Report frame sent");
    check(sent[0].first == mili::protocol::kCanIdDecisionReport, "Decision report CAN ID");
}

void test_evaluate_benchmark_budget() {
    mili::DecisionEngine engine(0);
    mili::SensorInput input{};
    input.battery_level = 70.0f;
    input.threat_level = 0.3f;
    input.target_visibility = 0.8f;

    const auto bench = mili::benchmark_evaluate(engine, input, 200);
    check(bench.passed, "evaluate() should complete within 5 ms budget on host");
    check(bench.max_evaluate_ms < bench.budget_ms, "Max latency under budget");
}

void test_hal_stubs() {
    mili::platform::CanHal can;
    std::uint32_t tx_count = 0;
    can.set_send_callback(
        [](std::uint16_t, const void*, std::size_t, void* ctx) -> bool {
            auto* count = static_cast<std::uint32_t*>(ctx);
            ++(*count);
            return true;
        },
        &tx_count);

    std::uint8_t payload[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    check(can.send(mili::protocol::kCanIdModeCommand, payload, sizeof(payload)), "CAN HAL send");
    check(tx_count == 1, "CAN HAL tx count");

    mili::platform::UartHal uart;
    char buffer[32] = {};
    std::size_t offset = 0;
    uart.set_write_callback(
        [](const char* data, std::size_t len, void* ctx) {
            auto* state = static_cast<std::pair<char*, std::size_t*>*>(ctx);
            char* out = state->first;
            std::size_t& off = *state->second;
            for (std::size_t i = 0; i < len && off < 31; ++i) {
                out[off++] = data[i];
            }
        },
        &std::pair<char*, std::size_t*>{buffer, &offset});
    uart.write_line("MILI");
    check(std::strncmp(buffer, "MILI", 4) == 0, "UART HAL write");

    mili::platform::SpiHal spi;
    spi.set_transfer_callback(
        [](std::uint8_t*, std::uint8_t* rx, std::size_t, void*) -> bool {
            rx[0] = 0x01;
            rx[1] = 50;
            rx[2] = 200;
            rx[3] = 180;
            rx[4] = 30;
            return true;
        },
        nullptr);

    mili::SensorInput input{};
    check(spi.apply_to_sensor_input(input), "SPI HAL environmental read");
    check(input.threat_level > 0.0f, "SPI threat applied");
}

void test_embedded_app_tick() {
    mili::SensorAdapter adapter;
    std::vector<std::pair<std::uint16_t, std::size_t>> sent;
    mili::CanControlPublisher control([&](std::uint16_t id, const void*, std::size_t len) {
        sent.emplace_back(id, len);
        return true;
    });
    mili::CanReportPublisher report(test_send_frame, &sent);
    mili::IntegrationHub hub(adapter, control);
    mili::DecisionEngine engine(0);

    mili::embedded::AppContext ctx(control, report, hub, engine);
    mili::embedded::app_init(ctx);

    mili::protocol::Product1CanPayload p1{};
    p1.battery_level_x100 = 8000;
    mili::embedded::app_on_can_frame(
        ctx,
        mili::protocol::kCanIdProduct1Status,
        reinterpret_cast<const std::uint8_t*>(&p1),
        sizeof(p1));

    const auto result = mili::embedded::app_tick(ctx, 1000);
    check(result.selected_score > 0.0f, "Embedded app tick should decide");
    check(sent.size() >= 2, "App tick should publish mode command and decision report");
}

}  // namespace

int main() {
    test_embedded_logger_no_heap();
    test_decision_report_protocol();
    test_config_update_protocol();
    test_can_router_and_report();
    test_evaluate_benchmark_budget();
    test_hal_stubs();
    test_embedded_app_tick();

    if (failures > 0) {
        std::cerr << failures << " embedded test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All embedded tests passed.\n";
    return EXIT_SUCCESS;
}

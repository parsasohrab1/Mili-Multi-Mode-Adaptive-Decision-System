#include "mili/hil/hil_runner.hpp"
#include "mili/hil/soak_runner.hpp"
#include "mili/integration/control_publisher.hpp"
#include "mili/platform/benchmark.hpp"
#include "mili/platform/can_router.hpp"
#include "mili/protocol/messages.hpp"

#include "mili_app.hpp"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

struct FrameCapture {
    std::vector<std::pair<std::uint16_t, std::size_t>> frames;
};

bool capture_send(std::uint16_t can_id, const void* data, std::size_t len, void* user_data) {
    auto* capture = static_cast<FrameCapture*>(user_data);
    capture->frames.emplace_back(can_id, len);
    (void)data;
    return true;
}

std::uint64_t soak_cycle_count() {
    const char* env = std::getenv("MILI_SOAK_CYCLES");
    if (env != nullptr && env[0] != '\0') {
        return static_cast<std::uint64_t>(std::strtoull(env, nullptr, 10));
    }
    return mili::hil::kSoakCyclesCi;
}

// G1 — replay field-style UAV mission scenarios.
void test_field_scenario_replay() {
    const char* scenarios[] = {
        "data/hil_scenarios/baseline.csv",
        "data/hil_scenarios/patrol_surveillance.csv",
        "data/hil_scenarios/threat_engagement.csv",
        "data/hil_scenarios/low_battery_return.csv",
        "data/hil_scenarios/hysteresis_switch.csv",
    };

    for (const char* path : scenarios) {
        FrameCapture capture{};
        mili::hil::HardwareMock mock(capture_send, &capture);
        mili::hil::HilRunner runner;
        check(runner.load_scenario_csv(path), path);
        const auto result = runner.run(mock, 100);
        check(result.events_processed > 0, "Field scenario events processed");
        check(result.cycles_executed > 0, "Field scenario cycles executed");
        check(result.control_frames_sent, "Field scenario publishes control");
        check(result.final_confidence > 0.0f, "Field scenario produces confidence");
        check(capture.frames.size() >= 2, "Field scenario emits CAN control frames");
    }
}

// G3 — end-to-end: CAN sensor ingress → IntegrationHub → mode + control params.
void test_e2e_sensor_to_control() {
    FrameCapture capture{};
    mili::hil::HardwareMock mock(capture_send, &capture);
    mili::hil::HilRunner runner;
    check(runner.load_scenario_csv("data/hil_scenarios/threat_engagement.csv"), "E2E scenario load");

    const auto result = runner.run(mock, 100);
    check(result.control_frames_sent, "E2E control path active");

    bool saw_mode = false;
    bool saw_params = false;
    for (const auto& frame : capture.frames) {
        if (frame.first == mili::protocol::kCanIdModeCommand) {
            saw_mode = true;
        }
        if (frame.first == mili::protocol::kCanIdControlParams) {
            saw_params = true;
        }
    }
    check(saw_mode, "E2E emits mode command (0x310)");
    check(saw_params, "E2E emits control parameters (0x313)");
    check(result.max_cycle_latency_us <= mili::hil::kFr2MaxSwitchPathUs,
        "E2E cycle latency within FR-2 budget");
}

// G5 — FR-2 switch path latency < 50 ms (host proxy for hardware gate).
void test_fr2_switch_path_latency() {
    FrameCapture capture{};
    mili::hil::HardwareMock mock(capture_send, &capture);
    mili::hil::HilRunner runner;
    check(runner.load_scenario_csv("data/hil_scenarios/hysteresis_switch.csv"), "Hysteresis scenario");

    const auto result = runner.run(mock, 100);
    std::cout << "FR-2 max cycle latency: " << (result.max_cycle_latency_us / 1000.0) << " ms\n";
    if (result.mode_switch_count > 0) {
        std::cout << "FR-2 max switch-path latency: "
                  << (result.max_switch_path_latency_us / 1000.0) << " ms\n";
    }

    check(result.max_cycle_latency_us <= mili::hil::kFr2MaxSwitchPathUs,
        "FR-2: hub cycle must complete within 50 ms");
    if (result.max_switch_path_latency_us > 0) {
        check(result.max_switch_path_latency_us <= mili::hil::kFr2MaxSwitchPathUs,
            "FR-2: mode switch path must complete within 50 ms");
    }
}

// G2 — continuous soak (72 min CI equivalent; set MILI_SOAK_CYCLES=2592000 for full 72 h).
void test_soak_continuous() {
    mili::SensorAdapter adapter;
    FrameCapture capture{};
    mili::ControlSystemPublisher publisher(capture_send, &capture);
    mili::IntegrationHub hub(adapter, publisher);
    mili::DecisionEngine engine(0);
    engine.load_config("config/default_weights.json");

    const auto cycles = soak_cycle_count();
    std::cout << "Soak test cycles: " << cycles << "\n";

    mili::hil::SoakRunner soak;
    const auto result = soak.run(adapter, hub, engine, cycles, 100);
    std::cout << "Soak max cycle: " << (result.max_cycle_us / 1000.0) << " ms\n";

    check(result.cycles_completed == cycles, "Soak completed all cycles");
    check(result.unhealthy_cycles == 0, "Soak health monitor stable");
    check(result.max_cycle_us <= mili::hil::kFr2MaxSwitchPathUs, "Soak cycle latency within budget");
}

// G4 — embedded regression: app tick + protocol + benchmark budget.
void test_embedded_regression_path() {
    mili::SensorAdapter adapter;
    std::vector<std::pair<std::uint16_t, std::size_t>> sent;
    mili::CanControlPublisher control([&](std::uint16_t id, const void*, std::size_t len) {
        sent.emplace_back(id, len);
        return true;
    });
    mili::CanReportPublisher report(capture_send, &sent);
    mili::IntegrationHub hub(adapter, control);
    mili::DecisionEngine engine(0);

    mili::embedded::AppContext ctx(control, report, hub, engine);
    mili::embedded::app_init(ctx);

    mili::protocol::Product1CanPayload p1{};
    p1.battery_level_x100 = 7500;
    mili::embedded::app_on_can_frame(
        ctx,
        mili::protocol::kCanIdProduct1Status,
        reinterpret_cast<const std::uint8_t*>(&p1),
        sizeof(p1));

    const auto start = std::chrono::steady_clock::now();
    const auto result = mili::embedded::app_tick(ctx, 1000);
    const auto end = std::chrono::steady_clock::now();
    const auto latency_us = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

    check(result.selected_score > 0.0f, "Embedded regression: app tick decides");
    check(!sent.empty(), "Embedded regression: CAN output");
    check(latency_us <= mili::hil::kFr2MaxSwitchPathUs, "Embedded regression: tick latency");

    mili::SensorInput bench_input{};
    bench_input.battery_level = 70.0f;
    bench_input.threat_level = 0.3f;
    bench_input.target_visibility = 0.8f;
    const auto bench = mili::benchmark_evaluate(engine, bench_input, 100);
    check(bench.passed, "Embedded regression: evaluate budget");
}

}  // namespace

int main() {
    test_field_scenario_replay();
    test_e2e_sensor_to_control();
    test_fr2_switch_path_latency();
    test_soak_continuous();
    test_embedded_regression_path();

    if (failures > 0) {
        std::cerr << failures << " field QA test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All field QA tests passed.\n";
    return EXIT_SUCCESS;
}

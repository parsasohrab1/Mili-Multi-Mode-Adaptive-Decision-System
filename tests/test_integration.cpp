#include "mili/decision_engine.hpp"
#include "mili/decision_loop.hpp"
#include "mili/mock_products.hpp"
#include "mili/sensor_adapter.hpp"
#include "mili/types.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_sensor_adapter_mapping() {
    mili::SensorAdapter adapter;
    const std::uint64_t now = 1000;

    mili::MockProduct1 p1;
    mili::MockProduct2 p2;
    adapter.ingest_product1(p1.sample(now, 72.5f), now);
    adapter.ingest_product2(p2.sample(now, 8.2f), now);
    adapter.ingest_environmental(mili::mock_environmental(now), now);

    const auto input = adapter.to_sensor_input(now);
    check(std::fabs(input.battery_level - 72.5f) < 0.01f, "Product1 battery should map to SensorInput");
    check(std::fabs(input.distance_to_home_km - 8.2f) < 0.01f, "Product2 distance should map to SensorInput");
    check(input.threat_level > 0.0f, "Environmental threat should map to SensorInput");
}

void test_stale_product1_uses_last_valid() {
    mili::SensorAdapter adapter(500);
    mili::MockProduct1 p1;

    adapter.ingest_product1(p1.sample(1000, 55.0f), 1000);

    const auto fresh = adapter.to_sensor_input(1200);
    check(std::fabs(fresh.battery_level - 55.0f) < 0.01f, "Fresh product1 value should be used");

    const auto stale = adapter.to_sensor_input(2000);
    check(adapter.product1_is_stale(2000), "Product1 should be marked stale");
    check(std::fabs(stale.battery_level - 55.0f) < 0.01f, "Stale product1 should keep last valid battery");
}

void test_operator_priority_override() {
    mili::SensorAdapter adapter;
    adapter.ingest_environmental(mili::mock_environmental(0), 0);

    const auto default_input = adapter.to_sensor_input(0);
    check(default_input.mission_priority == mili::MissionPriority::Medium, "Default priority should be medium");

    mili::OperatorCommand command{};
    command.valid = true;
    command.has_priority_override = true;
    command.priority_override = mili::MissionPriority::Critical;
    adapter.ingest_operator(command, 100);

    const auto overridden = adapter.to_sensor_input(100);
    check(overridden.mission_priority == mili::MissionPriority::Critical, "Operator override should apply immediately");
}

void test_decision_loop_10hz_jitter() {
    mili::DecisionEngine engine(0);
    mili::SensorAdapter adapter;
    mili::DecisionLoop loop(engine, adapter, 10.0f);

    mili::MockProduct1 p1;
    mili::MockProduct2 p2;

    const std::uint64_t step_ms = 100;
    for (int i = 0; i < 100; ++i) {
        const std::uint64_t ts = static_cast<std::uint64_t>(i) * step_ms;
        adapter.ingest_product1(p1.sample(ts, 70.0f), ts);
        adapter.ingest_product2(p2.sample(ts, 5.0f), ts);
        adapter.ingest_environmental(mili::mock_environmental(ts), ts);
        loop.tick(ts);
    }

    const auto& stats = loop.stats();
    std::cout << "Loop avg jitter=" << stats.avg_jitter_ms << " ms, max=" << stats.max_jitter_ms << " ms\n";
    check(stats.tick_count == 100, "Loop should execute 100 ticks");
    check(stats.max_jitter_ms < 2.0f, "Loop jitter should remain below 2ms for uniform 10Hz ticks");
}

void test_operator_override_within_one_cycle() {
    mili::DecisionEngine engine(0);
    mili::SensorAdapter adapter;
    mili::DecisionLoop loop(engine, adapter, 10.0f);

    mili::MockProduct1 p1;
    mili::MockProduct2 p2;

    adapter.ingest_product1(p1.sample(0, 80.0f), 0);
    adapter.ingest_product2(p2.sample(0, 6.0f), 0);
    adapter.ingest_environmental(mili::mock_environmental(0), 0);
    loop.tick(0);

    mili::OperatorCommand command{};
    command.valid = true;
    command.has_priority_override = true;
    command.priority_override = mili::MissionPriority::Critical;
    adapter.ingest_operator(command, 100);

    const auto result = loop.tick(100);
    const auto input = adapter.to_sensor_input(100);

    check(input.mission_priority == mili::MissionPriority::Critical, "Override should be active on next tick");
    check(
        result.all_scores[static_cast<std::size_t>(mili::OperationalMode::Engagement)].score > 0.0f,
        "Critical override should influence decision scores within one cycle");
}

}  // namespace

int main() {
    test_sensor_adapter_mapping();
    test_stale_product1_uses_last_valid();
    test_operator_priority_override();
    test_decision_loop_10hz_jitter();
    test_operator_override_within_one_cycle();

    if (failures > 0) {
        std::cerr << failures << " integration test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All integration tests passed.\n";
    return EXIT_SUCCESS;
}

#include "mili/hil/soak_runner.hpp"

#include "mili/mock_products.hpp"
#include "mili/sensor_adapter.hpp"

#include <chrono>

namespace mili::hil {

SoakResult SoakRunner::run(
    SensorAdapter& adapter,
    IntegrationHub& hub,
    DecisionEngine& engine,
    const std::uint64_t cycles,
    const std::uint64_t step_ms,
    InputMutator mutator) {
    SoakResult result{};

    for (std::uint64_t i = 0; i < cycles; ++i) {
        const std::uint64_t now_ms = i * step_ms;

        auto env = mock_environmental(now_ms);
        SensorInput probe{};
        probe.battery_level = 55.0f + static_cast<float>((i % 40));
        probe.threat_level = 0.1f + static_cast<float>((i % 7)) * 0.1f;
        probe.comm_strength = 0.5f + static_cast<float>((i % 5)) * 0.08f;
        probe.target_visibility = 0.4f + static_cast<float>((i % 6)) * 0.08f;
        probe.distance_to_home_km = 2.0f + static_cast<float>((i % 10));
        if (mutator != nullptr) {
            mutator(probe, i, now_ms);
        }

        Product1Data p1{};
        p1.battery_level_percent = probe.battery_level;
        p1.valid = true;
        p1.timestamp_ms = now_ms;
        adapter.ingest_product1(p1, now_ms);

        Product2Data p2{};
        p2.distance_to_home_km = probe.distance_to_home_km;
        p2.valid = true;
        p2.timestamp_ms = now_ms;
        adapter.ingest_product2(p2, now_ms);

        env.threat_level = probe.threat_level;
        env.comm_strength = probe.comm_strength;
        env.target_visibility = probe.target_visibility;
        env.valid = true;
        adapter.ingest_environmental(env, now_ms);

        const auto start = std::chrono::steady_clock::now();
        const auto decision = hub.run_cycle(engine, now_ms);
        const auto end = std::chrono::steady_clock::now();

        const auto latency_us = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        if (latency_us > result.max_cycle_us) {
            result.max_cycle_us = latency_us;
        }

        if (!hub.is_healthy()) {
            ++result.unhealthy_cycles;
        }
        if (decision.emergency_forced) {
            ++result.emergency_forced_count;
        }

        ++result.cycles_completed;
    }

    result.passed = result.unhealthy_cycles == 0 && result.max_cycle_us <= kFr2MaxSwitchPathUs;
    return result;
}

}  // namespace mili::hil

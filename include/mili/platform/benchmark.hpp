#pragma once

#include "mili/decision_engine.hpp"
#include "mili/embedded_config.hpp"
#include "mili/types.hpp"

#include <cstdint>

namespace mili {

struct BenchmarkResult {
    std::uint32_t iterations = 0;
    float mean_evaluate_ms = 0.0f;
    float max_evaluate_ms = 0.0f;
    float budget_ms = 5.0f;
    bool passed = false;
};

class CycleCounter {
public:
    static void reset();
    static std::uint32_t cycles();
    static float cycles_to_ms(std::uint32_t cycles, std::uint32_t cpu_mhz = 480);
};

BenchmarkResult benchmark_evaluate(
    DecisionEngine& engine,
    const SensorInput& input,
    std::uint32_t iterations = 1000);

}  // namespace mili

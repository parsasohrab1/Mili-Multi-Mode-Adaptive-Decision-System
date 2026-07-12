#include "mili/platform/benchmark.hpp"

#include <algorithm>
#include <chrono>
#include <limits>

namespace mili {

void CycleCounter::reset() {
#if defined(MILI_STM32_DWT) && defined(MILI_EMBEDDED)
    extern void mili_dwt_reset();
    mili_dwt_reset();
#else
    // Host stub: timing handled by chrono in benchmark_evaluate.
#endif
}

std::uint32_t CycleCounter::cycles() {
#if defined(MILI_STM32_DWT) && defined(MILI_EMBEDDED)
    extern std::uint32_t mili_dwt_cycles();
    return mili_dwt_cycles();
#else
    return 0;
#endif
}

float CycleCounter::cycles_to_ms(std::uint32_t cycles, std::uint32_t cpu_mhz) {
    if (cpu_mhz == 0) {
        return 0.0f;
    }
    return static_cast<float>(cycles) / static_cast<float>(cpu_mhz * 1000u);
}

BenchmarkResult benchmark_evaluate(
    DecisionEngine& engine, const SensorInput& input, std::uint32_t iterations) {
    BenchmarkResult result{};
    result.iterations = iterations;
    result.budget_ms = 5.0f;
#ifdef MILI_EMBEDDED
    result.budget_ms = kEmbeddedEvaluateBudgetMs;
#endif

    if (iterations == 0) {
        return result;
    }

    float max_ms = 0.0f;
    double total_ms = 0.0;

    for (std::uint32_t i = 0; i < iterations; ++i) {
        const auto start = std::chrono::steady_clock::now();
        engine.evaluate(input, static_cast<std::uint64_t>(i));
        const auto end = std::chrono::steady_clock::now();

        const float elapsed_ms = std::chrono::duration<float, std::milli>(end - start).count();
        total_ms += elapsed_ms;
        max_ms = std::max(max_ms, elapsed_ms);
    }

    result.mean_evaluate_ms = static_cast<float>(total_ms / static_cast<double>(iterations));
    result.max_evaluate_ms = max_ms;
    result.passed = result.max_evaluate_ms < result.budget_ms;
    return result;
}

}  // namespace mili

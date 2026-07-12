#pragma once

#include "mili/integration/integration_hub.hpp"
#include "mili/types.hpp"

#include <cstdint>
#include <functional>

namespace mili::hil {

// SRS FR-2: mode command path latency budget (sensor ingest → CAN publish).
constexpr std::uint64_t kFr2MaxSwitchPathUs = 50'000;

// G2 soak: 72 h @ 10 Hz = 2_592_000 cycles; CI uses 72 min equivalent.
constexpr std::uint64_t kSoakCycles72h = 2'592'000;
constexpr std::uint64_t kSoakCyclesCi = 25'920;

struct SoakResult {
    std::uint64_t cycles_completed;
    std::uint64_t unhealthy_cycles;
    std::uint64_t emergency_forced_count;
    std::uint64_t max_cycle_us;
    bool passed;
};

class SoakRunner {
public:
    using InputMutator = void (*)(SensorInput& input, std::uint64_t cycle, std::uint64_t now_ms);

    SoakResult run(
        SensorAdapter& adapter,
        IntegrationHub& hub,
        DecisionEngine& engine,
        std::uint64_t cycles,
        std::uint64_t step_ms,
        InputMutator mutator = nullptr);
};

}  // namespace mili::hil

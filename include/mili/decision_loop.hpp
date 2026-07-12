#pragma once

#include "mili/decision_engine.hpp"
#include "mili/sensor_adapter.hpp"
#include "mili/types.hpp"

namespace mili {

struct LoopStats {
    int tick_count = 0;
    float avg_jitter_ms = 0.0f;
    float max_jitter_ms = 0.0f;
};

class DecisionLoop {
public:
    DecisionLoop(DecisionEngine& engine, SensorAdapter& adapter, float rate_hz = kUpdateRateHz);

    DecisionResult tick(std::uint64_t timestamp_ms);
    float rate_hz() const;
    const LoopStats& stats() const;

private:
    DecisionEngine* engine_;
    SensorAdapter* adapter_;
    float rate_hz_;
    std::uint64_t expected_interval_ms_;
    std::uint64_t last_tick_ms_;
    LoopStats stats_;
};

}  // namespace mili

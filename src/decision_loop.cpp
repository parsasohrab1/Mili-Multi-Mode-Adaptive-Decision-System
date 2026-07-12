#include "mili/decision_loop.hpp"

#include <cmath>

namespace mili {

DecisionLoop::DecisionLoop(DecisionEngine& engine, SensorAdapter& adapter, float rate_hz)
    : engine_(&engine)
    , adapter_(&adapter)
    , rate_hz_(rate_hz)
    , expected_interval_ms_(static_cast<std::uint64_t>(1000.0f / rate_hz))
    , last_tick_ms_(0) {}

DecisionResult DecisionLoop::tick(std::uint64_t timestamp_ms) {
    if (last_tick_ms_ != 0) {
        const auto actual_interval = timestamp_ms - last_tick_ms_;
        const float jitter = std::fabs(static_cast<float>(actual_interval)
            - static_cast<float>(expected_interval_ms_));
        stats_.max_jitter_ms = std::max(stats_.max_jitter_ms, jitter);
        stats_.avg_jitter_ms = ((stats_.avg_jitter_ms * stats_.tick_count) + jitter)
            / static_cast<float>(stats_.tick_count + 1);
    }

    ++stats_.tick_count;
    last_tick_ms_ = timestamp_ms;

    const auto input = adapter_->to_sensor_input(timestamp_ms);
    return engine_->evaluate(input, timestamp_ms);
}

float DecisionLoop::rate_hz() const {
    return rate_hz_;
}

const LoopStats& DecisionLoop::stats() const {
    return stats_;
}

}  // namespace mili

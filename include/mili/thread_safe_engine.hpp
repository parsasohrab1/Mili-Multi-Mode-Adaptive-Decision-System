#pragma once

#include "mili/decision_engine.hpp"
#include "mili/types.hpp"

#include <mutex>

namespace mili {

class ThreadSafeDecisionEngine {
public:
    explicit ThreadSafeDecisionEngine(DecisionEngine& engine);

    DecisionResult evaluate(
        const SensorInput& input,
        std::uint64_t timestamp_ms,
        const EvaluateContext& context = {});

    OperationalMode current_mode() const;
    float last_confidence() const;

private:
    DecisionEngine& engine_;
    mutable std::mutex mutex_;
};

}  // namespace mili

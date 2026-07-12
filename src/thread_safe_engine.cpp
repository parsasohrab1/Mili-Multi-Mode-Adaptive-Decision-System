#include "mili/thread_safe_engine.hpp"

namespace mili {

ThreadSafeDecisionEngine::ThreadSafeDecisionEngine(DecisionEngine& engine)
    : engine_(engine) {}

DecisionResult ThreadSafeDecisionEngine::evaluate(
    const SensorInput& input, std::uint64_t timestamp_ms, const EvaluateContext& context) {
    const std::lock_guard<std::mutex> lock(mutex_);
    return engine_.evaluate(input, timestamp_ms, context);
}

OperationalMode ThreadSafeDecisionEngine::current_mode() const {
    const std::lock_guard<std::mutex> lock(mutex_);
    return engine_.current_mode();
}

float ThreadSafeDecisionEngine::last_confidence() const {
    const std::lock_guard<std::mutex> lock(mutex_);
    return engine_.last_confidence();
}

}  // namespace mili

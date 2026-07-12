#include "mili/low_confidence_policy.hpp"

#include <algorithm>

namespace mili {

OperationalMode apply_low_confidence_policy(OperationalMode candidate, float confidence) {
    if (confidence >= kLowConfidenceThreshold) {
        return candidate;
    }
    if (candidate == OperationalMode::Engagement || candidate == OperationalMode::Pursuit) {
        return OperationalMode::Reconnaissance;
    }
    if (candidate == OperationalMode::Surveillance) {
        return OperationalMode::Loiter;
    }
    return candidate;
}

void apply_low_confidence_parameters(ModeParameters& params, float confidence) {
    if (confidence >= kLowConfidenceThreshold) {
        return;
    }
    params.sensor_aggressiveness *= 0.5f;
    params.engagement_readiness *= 0.5f;
    params.speed_factor = std::min(params.speed_factor, 0.5f);
}

}  // namespace mili

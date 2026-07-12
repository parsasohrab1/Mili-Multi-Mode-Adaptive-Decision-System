#include "mili/mode_parameters.hpp"

#include <algorithm>

namespace mili {

ModeParameters ModeParameterMapper::for_mode(OperationalMode mode, const SensorInput& input, float mode_score) {
    ModeParameters params{};
    const float battery_norm = std::max(0.0f, std::min(1.0f, input.battery_level / 100.0f));
    const float score_norm = std::max(0.0f, std::min(1.0f, mode_score));

    switch (mode) {
        case OperationalMode::Reconnaissance:
            params.speed_factor = 0.6f + 0.3f * battery_norm;
            params.altitude_factor = 0.7f;
            params.sensor_aggressiveness = 0.8f;
            params.engagement_readiness = 0.1f;
            params.loiter_radius_km = 1.0f;
            break;
        case OperationalMode::Surveillance:
            params.speed_factor = 0.3f;
            params.altitude_factor = 0.6f + 0.2f * input.target_visibility;
            params.sensor_aggressiveness = 0.9f;
            params.engagement_readiness = 0.2f;
            params.loiter_radius_km = 0.8f;
            break;
        case OperationalMode::Pursuit:
            params.speed_factor = 0.8f + 0.2f * score_norm;
            params.altitude_factor = 0.4f;
            params.sensor_aggressiveness = 0.7f;
            params.engagement_readiness = 0.5f;
            params.loiter_radius_km = 0.3f;
            break;
        case OperationalMode::Loiter:
            params.speed_factor = 0.2f;
            params.altitude_factor = 0.5f;
            params.sensor_aggressiveness = 0.4f;
            params.engagement_readiness = 0.15f;
            params.loiter_radius_km = 0.5f + input.threat_level * 0.5f;
            break;
        case OperationalMode::ReturnHome:
            params.speed_factor = 0.5f + 0.4f * (1.0f - battery_norm);
            params.altitude_factor = 0.5f;
            params.sensor_aggressiveness = 0.2f;
            params.engagement_readiness = 0.0f;
            params.return_urgency = std::max(0.5f, 1.0f - battery_norm);
            params.loiter_radius_km = 0.0f;
            break;
        case OperationalMode::Engagement:
            params.speed_factor = 0.9f;
            params.altitude_factor = 0.3f;
            params.sensor_aggressiveness = 1.0f;
            params.engagement_readiness = 0.7f + 0.3f * input.threat_level;
            params.loiter_radius_km = 0.0f;
            break;
        default:
            break;
    }

    return params;
}

}  // namespace mili

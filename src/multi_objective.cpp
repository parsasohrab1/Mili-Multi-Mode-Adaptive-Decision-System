#include "mili/multi_objective.hpp"

#include <algorithm>

namespace mili {

MultiObjectiveEvaluator::MultiObjectiveEvaluator(ObjectiveWeights weights)
    : weights_(weights) {}

ObjectiveVector MultiObjectiveEvaluator::evaluate_mode(OperationalMode mode, const SensorInput& input) const {
    const float battery = std::max(0.0f, std::min(1.0f, input.battery_level / 100.0f));
    const float threat_inv = 1.0f - input.threat_level;
    const float visibility = input.target_visibility;
    const float comm = input.comm_strength;
    const float distance_inv = 1.0f - (input.distance_to_home_km / kMaxDistanceKm);

    ObjectiveVector obj{};
    switch (mode) {
        case OperationalMode::Reconnaissance:
            obj.energy = 0.7f * battery + 0.3f * comm;
            obj.mission_success = 0.6f * visibility + 0.4f * threat_inv;
            obj.survival = 0.5f * battery + 0.5f * threat_inv;
            break;
        case OperationalMode::Pursuit:
            obj.energy = 0.25f * battery + 0.25f * comm;
            obj.mission_success = 0.55f * visibility + 0.25f * comm
                + (input.mission_priority >= MissionPriority::High ? 0.15f : 0.0f);
            obj.survival = 0.45f * threat_inv + 0.35f * battery + 0.20f * comm;
            break;
        case OperationalMode::Surveillance:
            obj.energy = 0.35f * (1.0f - battery) + 0.35f * comm;
            obj.mission_success = 0.75f * visibility + 0.15f * comm;
            if (battery > 0.55f && comm > 0.6f) {
                obj.mission_success *= 0.85f;
            }
            obj.survival = 0.6f * threat_inv + 0.4f * battery;
            break;
        case OperationalMode::Loiter:
            obj.energy = 0.8f * battery;
            obj.mission_success = 0.4f * visibility + 0.6f * comm;
            obj.survival = 0.7f * threat_inv + 0.3f * (1.0f - visibility);
            break;
        case OperationalMode::ReturnHome:
            obj.energy = 1.0f - battery;
            obj.mission_success = 0.2f * visibility;
            obj.survival = 0.6f * threat_inv + 0.4f * distance_inv;
            break;
        case OperationalMode::Engagement:
            obj.energy = 0.2f * battery;
            obj.mission_success = 0.7f * visibility + 0.3f * input.threat_level;
            obj.survival = 0.4f * battery + 0.6f * threat_inv;
            break;
        default:
            break;
    }
    return obj;
}

float MultiObjectiveEvaluator::scalar_score(const ObjectiveVector& objectives) const {
    return weights_.energy * objectives.energy
         + weights_.mission_success * objectives.mission_success
         + weights_.survival * objectives.survival;
}

std::array<float, kModeCount> MultiObjectiveEvaluator::score_all_modes(const SensorInput& input) const {
    std::array<float, kModeCount> scores{};
    for (std::size_t i = 0; i < kModeCount; ++i) {
        const auto mode = static_cast<OperationalMode>(i);
        scores[i] = scalar_score(evaluate_mode(mode, input));
    }
    return scores;
}

}  // namespace mili

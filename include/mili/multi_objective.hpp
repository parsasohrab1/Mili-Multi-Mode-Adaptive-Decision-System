#pragma once

#include "mili/types.hpp"

namespace mili {

enum class ObjectiveId : std::uint8_t {
    Energy = 0,
    MissionSuccess,
    Survival,
    Count
};

struct ObjectiveVector {
    float energy = 0.0f;
    float mission_success = 0.0f;
    float survival = 0.0f;
};

struct ObjectiveWeights {
    float energy = 0.30f;
    float mission_success = 0.40f;
    float survival = 0.30f;
};

class MultiObjectiveEvaluator {
public:
    explicit MultiObjectiveEvaluator(ObjectiveWeights weights = {});

    ObjectiveVector evaluate_mode(OperationalMode mode, const SensorInput& input) const;
    float scalar_score(const ObjectiveVector& objectives) const;
    std::array<float, kModeCount> score_all_modes(const SensorInput& input) const;

private:
    ObjectiveWeights weights_;
};

}  // namespace mili

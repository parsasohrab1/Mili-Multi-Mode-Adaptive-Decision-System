#pragma once

#include "mili/data_quality_monitor.hpp"
#include "mili/types.hpp"

namespace mili {

struct ImputationWeights {
    float battery = 1.0f;
    float navigation = 1.0f;
    float environmental = 1.0f;
    float combined = 1.0f;
};

struct ImputedInput {
    SensorInput input{};
    ImputationWeights weights{};
};

class SensorImputer {
public:
    static ImputedInput apply(const SensorInput& raw, const DataQualityStatus& quality);

    static void scale_scores(std::array<float, kModeCount>& scores, const ImputationWeights& weights);
};

}  // namespace mili

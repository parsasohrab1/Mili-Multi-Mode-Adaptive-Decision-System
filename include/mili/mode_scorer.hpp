#pragma once

#include "mili/factor_normalizer.hpp"
#include "mili/types.hpp"
#include "mili/weight_config.hpp"
#include "mili/weight_manager.hpp"

#include <array>

namespace mili {

class ModeScorer {
public:
    explicit ModeScorer(const WeightManager* weight_manager = nullptr);

    std::array<float, kModeCount> compute_scores(const SensorInput& input) const;

private:
    const WeightManager* weight_manager_;

    const WeightConfig& weights() const;

    float score_reconnaissance(const NormalizedFactors& factors) const;
    float score_surveillance(const NormalizedFactors& factors) const;
    float score_pursuit(const NormalizedFactors& factors) const;
    float score_loiter(const NormalizedFactors& factors) const;
    float score_return_home(const NormalizedFactors& factors) const;
    float score_engagement(const NormalizedFactors& factors) const;

    void apply_priority_multipliers(MissionPriority priority, std::array<float, kModeCount>& scores) const;
    void apply_environmental_factors(const SensorInput& input, std::array<float, kModeCount>& scores) const;
};

}  // namespace mili

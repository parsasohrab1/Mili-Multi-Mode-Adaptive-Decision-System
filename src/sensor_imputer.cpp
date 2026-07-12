#include "mili/sensor_imputer.hpp"

#include <algorithm>

namespace mili {

namespace {

float quality_weight(SourceQuality quality) {
    switch (quality) {
        case SourceQuality::Ok:
            return 1.0f;
        case SourceQuality::Stale:
            return 0.75f;
        case SourceQuality::Invalid:
            return 0.5f;
        case SourceQuality::Missing:
        default:
            return 0.4f;
    }
}

}  // namespace

ImputedInput SensorImputer::apply(const SensorInput& raw, const DataQualityStatus& quality) {
    ImputedInput result{};
    result.input = raw;
    result.weights.battery = quality_weight(quality.product1_quality);
    result.weights.navigation = quality_weight(quality.product2_quality);
    result.weights.environmental = quality_weight(quality.environmental_quality);
    result.weights.combined = quality.imputation_factor;

    if (quality.product1_quality == SourceQuality::Missing
        || quality.product1_quality == SourceQuality::Invalid) {
        result.input.battery_level = 50.0f;
    }

    if (quality.product2_quality == SourceQuality::Missing
        || quality.product2_quality == SourceQuality::Invalid) {
        result.input.distance_to_home_km = 5.0f;
    }

    if (quality.environmental_quality == SourceQuality::Missing
        || quality.environmental_quality == SourceQuality::Invalid) {
        result.input.threat_level = 0.5f;
        result.input.comm_strength = 0.5f;
        result.input.target_visibility = 0.5f;
    } else if (quality.environmental_quality == SourceQuality::Stale) {
        result.input.threat_level = std::min(1.0f, raw.threat_level * 1.1f);
    }

    return result;
}

void SensorImputer::scale_scores(std::array<float, kModeCount>& scores, const ImputationWeights& weights) {
    const float scale = weights.combined
        * ((weights.battery + weights.navigation + weights.environmental) / 3.0f);
    for (auto& score : scores) {
        score *= scale;
    }
}

}  // namespace mili

#include "mili/factor_normalizer.hpp"

#include <algorithm>
#include <cmath>

namespace mili {

float FactorNormalizer::clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

NormalizedFactors FactorNormalizer::normalize(const SensorInput& input) {
    NormalizedFactors factors{};
    factors.battery = clamp01(input.battery_level / 100.0f);
    factors.battery_inverse = 1.0f - factors.battery;
    factors.threat = clamp01(input.threat_level);
    factors.threat_inverse = 1.0f - factors.threat;
    factors.comm = clamp01(input.comm_strength);
    factors.comm_inverse = 1.0f - factors.comm;
    factors.visibility = clamp01(input.target_visibility);
    factors.visibility_inverse = 1.0f - factors.visibility;
    factors.distance_inverse = clamp01(1.0f - (input.distance_to_home_km / kMaxDistanceKm));
    factors.distance_close = input.distance_to_home_km < 3.0f ? 1.0f : 0.0f;
    factors.wind_inverse = clamp01(1.0f - (input.wind_speed_mps / kMaxWindSpeedMps));
    factors.battery_ok_40 = input.battery_level > 40.0f ? 1.0f : 0.0f;
    factors.battery_ok_50 = input.battery_level > 50.0f ? 1.0f : 0.0f;
    factors.threat_high_60 = input.threat_level > 0.6f ? 1.0f : 0.0f;
    factors.threat_high_70 = input.threat_level > 0.7f ? 1.0f : 0.0f;
    factors.comm_ok = input.comm_strength > 0.5f ? 1.0f : 0.0f;
    factors.priority_critical = input.mission_priority == MissionPriority::Critical ? 1.0f : 0.0f;
    factors.distance_km = input.distance_to_home_km;
    return factors;
}

float FactorNormalizer::weighted_sum(const float* weights, const float* factors, std::size_t count) {
    float total = 0.0f;
    for (std::size_t i = 0; i < count; ++i) {
        total += weights[i] * factors[i];
    }
    return total;
}

}  // namespace mili

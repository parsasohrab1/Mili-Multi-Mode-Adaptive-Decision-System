#pragma once

#include "mili/types.hpp"

namespace mili {

struct NormalizedFactors {
    float battery = 0.0f;
    float battery_inverse = 0.0f;
    float threat = 0.0f;
    float threat_inverse = 0.0f;
    float comm = 0.0f;
    float comm_inverse = 0.0f;
    float visibility = 0.0f;
    float visibility_inverse = 0.0f;
    float distance_inverse = 0.0f;
    float distance_close = 0.0f;
    float wind_inverse = 0.0f;
    float battery_ok_40 = 0.0f;
    float battery_ok_50 = 0.0f;
    float threat_high_60 = 0.0f;
    float threat_high_70 = 0.0f;
    float comm_ok = 0.0f;
    float priority_critical = 0.0f;
    float distance_km = 0.0f;
};

class FactorNormalizer {
public:
    static float clamp01(float value);
    static NormalizedFactors normalize(const SensorInput& input);
    static float weighted_sum(const float* weights, const float* factors, std::size_t count);
};

}  // namespace mili

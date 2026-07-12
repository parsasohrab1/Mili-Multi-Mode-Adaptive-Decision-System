#pragma once

#include "mili/types.hpp"

#include <cstddef>

namespace mili {

struct FourTermModeWeights {
    float terms[4]{};
};

struct EngagementModeWeights {
    float target_visibility = 0.5f;
    float threat_threshold = 0.3f;
    float battery_threshold = 0.1f;
    float priority_boost = 0.2f;
    float distance_penalty = 0.02f;
};

struct PriorityMultiplierSet {
    float engagement = 1.0f;
    float surveillance = 1.0f;
    float loiter = 1.0f;
    float return_home = 1.0f;
    float reconnaissance = 1.0f;
};

struct EnvironmentalMultipliers {
    float rain_surveillance = 0.7f;
    float rain_pursuit = 0.7f;
    float fog_reconnaissance = 0.6f;
    float fog_surveillance = 0.6f;
    float night_reconnaissance = 0.5f;
    float night_surveillance = 0.7f;
};

struct WeightConfig {
    FourTermModeWeights reconnaissance;
    FourTermModeWeights surveillance;
    FourTermModeWeights pursuit;
    FourTermModeWeights loiter;
    FourTermModeWeights return_home;
    EngagementModeWeights engagement;
    PriorityMultiplierSet critical;
    PriorityMultiplierSet low;
    EnvironmentalMultipliers environmental;
    float hysteresis_seconds = kHysteresisSeconds;
    float update_rate_hz = kUpdateRateHz;
    std::size_t max_log_entries = kMaxLogEntries;
};

WeightConfig make_default_weight_config();

}  // namespace mili

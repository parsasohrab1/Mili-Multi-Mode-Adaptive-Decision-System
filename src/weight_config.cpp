#include "mili/weight_config.hpp"

namespace mili {

WeightConfig make_default_weight_config() {
    WeightConfig config{};

    config.reconnaissance.terms[0] = 0.4f;
    config.reconnaissance.terms[1] = 0.3f;
    config.reconnaissance.terms[2] = 0.2f;
    config.reconnaissance.terms[3] = 0.1f;

    config.surveillance.terms[0] = 0.5f;
    config.surveillance.terms[1] = 0.2f;
    config.surveillance.terms[2] = 0.2f;
    config.surveillance.terms[3] = 0.1f;

    config.pursuit.terms[0] = 0.6f;
    config.pursuit.terms[1] = 0.2f;
    config.pursuit.terms[2] = 0.1f;
    config.pursuit.terms[3] = 0.1f;

    config.loiter.terms[0] = 0.3f;
    config.loiter.terms[1] = 0.3f;
    config.loiter.terms[2] = 0.2f;
    config.loiter.terms[3] = 0.2f;

    config.return_home.terms[0] = 0.4f;
    config.return_home.terms[1] = 0.3f;
    config.return_home.terms[2] = 0.2f;
    config.return_home.terms[3] = 0.1f;

    config.engagement.target_visibility = 0.5f;
    config.engagement.threat_threshold = 0.3f;
    config.engagement.battery_threshold = 0.1f;
    config.engagement.priority_boost = 0.2f;
    config.engagement.distance_penalty = 0.02f;

    config.critical.engagement = 1.3f;
    config.critical.surveillance = 1.1f;
    config.critical.loiter = 1.0f;
    config.critical.return_home = 1.0f;
    config.critical.reconnaissance = 1.0f;

    config.low.loiter = 1.3f;
    config.low.return_home = 1.2f;
    config.low.reconnaissance = 0.8f;
    config.low.engagement = 1.0f;
    config.low.surveillance = 1.0f;

    config.environmental.rain_surveillance = 0.7f;
    config.environmental.rain_pursuit = 0.7f;
    config.environmental.fog_reconnaissance = 0.6f;
    config.environmental.fog_surveillance = 0.6f;
    config.environmental.night_reconnaissance = 0.5f;
    config.environmental.night_surveillance = 0.7f;

    config.hysteresis_seconds = kHysteresisSeconds;
    config.update_rate_hz = kUpdateRateHz;
    config.max_log_entries = kMaxLogEntries;

    return config;
}

}  // namespace mili

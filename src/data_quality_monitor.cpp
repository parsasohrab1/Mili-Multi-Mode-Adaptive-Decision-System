#include "mili/data_quality_monitor.hpp"

#include <cmath>

namespace mili {

DataQualityMonitor::DataQualityMonitor(std::uint64_t stale_threshold_ms)
    : stale_threshold_ms_(stale_threshold_ms)
    , min_missing_for_emergency_(2) {}

bool DataQualityMonitor::is_stale(
    std::uint64_t timestamp_ms, std::uint64_t now_ms, std::uint64_t threshold_ms) {
    return now_ms > timestamp_ms + threshold_ms;
}

bool DataQualityMonitor::is_valid_product1(const Product1Data& data) {
    if (!data.valid) {
        return false;
    }
    if (data.battery_level_percent < 0.0f || data.battery_level_percent > 100.0f) {
        return false;
    }
    if (data.power_draw_watts < 0.0f || data.power_budget_watts < 0.0f) {
        return false;
    }
    return true;
}

bool DataQualityMonitor::is_valid_product2(const Product2Data& data) {
    if (!data.valid) {
        return false;
    }
    if (data.distance_to_home_km < 0.0f || data.velocity_mps < 0.0f) {
        return false;
    }
    if (data.vio_confidence > 0.0f && data.vio_confidence < 0.2f) {
        return false;
    }
    return true;
}

bool DataQualityMonitor::is_valid_environmental(const EnvironmentalData& data) {
    if (!data.valid) {
        return false;
    }
    if (data.threat_level < 0.0f || data.threat_level > 1.0f) {
        return false;
    }
    if (data.comm_strength < 0.0f || data.comm_strength > 1.0f) {
        return false;
    }
    if (data.target_visibility < 0.0f || data.target_visibility > 1.0f) {
        return false;
    }
    if (data.wind_speed_mps < 0.0f || data.wind_speed_mps > kMaxWindSpeedMps) {
        return false;
    }
    return true;
}

SourceQuality DataQualityMonitor::classify(bool present, bool stale, bool invalid) {
    if (!present) {
        return SourceQuality::Missing;
    }
    if (invalid) {
        return SourceQuality::Invalid;
    }
    if (stale) {
        return SourceQuality::Stale;
    }
    return SourceQuality::Ok;
}

DataQualityStatus DataQualityMonitor::evaluate(
    const Product1Data& p1,
    const Product2Data& p2,
    const EnvironmentalData& env,
    const OperatorCommand& op,
    std::uint64_t now_ms) const {
    DataQualityStatus status{};

    const bool p1_present = p1.valid;
    const bool p2_present = p2.valid;
    const bool env_present = env.valid;
    const bool op_present = op.valid;

    status.product1_valid = p1_present && is_valid_product1(p1);
    status.product2_valid = p2_present && is_valid_product2(p2);
    status.environmental_valid = env_present && is_valid_environmental(env);
    status.operator_valid = op_present;

    status.product1_stale = p1_present && is_stale(p1.timestamp_ms, now_ms, stale_threshold_ms_);
    status.product2_stale = p2_present && is_stale(p2.timestamp_ms, now_ms, stale_threshold_ms_);
    status.environmental_stale = env_present && is_stale(env.timestamp_ms, now_ms, stale_threshold_ms_);

    status.product1_invalid = p1_present && !is_valid_product1(p1);
    status.product2_invalid = p2_present && !is_valid_product2(p2);
    status.environmental_invalid = env_present && !is_valid_environmental(env);

    status.product1_quality = classify(p1_present, status.product1_stale, status.product1_invalid);
    status.product2_quality = classify(p2_present, status.product2_stale, status.product2_invalid);
    status.environmental_quality = classify(env_present, status.environmental_stale, status.environmental_invalid);
    status.operator_quality = op_present ? SourceQuality::Ok : SourceQuality::Missing;

    const SourceQuality critical_sources[] = {
        status.product1_quality,
        status.product2_quality,
        status.environmental_quality,
    };

    for (const auto quality : critical_sources) {
        switch (quality) {
            case SourceQuality::Missing:
                ++status.missing_count;
                ++status.degraded_count;
                break;
            case SourceQuality::Stale:
                ++status.stale_count;
                ++status.degraded_count;
                break;
            case SourceQuality::Invalid:
                ++status.invalid_count;
                ++status.degraded_count;
                break;
            default:
                break;
        }
    }

    status.degraded_inputs = status.degraded_count > 0;

    if (status.degraded_count >= min_missing_for_emergency_) {
        status.emergency_return = true;
        status.imputation_factor = 0.5f;
    } else if (status.degraded_count == 1) {
        status.imputation_factor = 0.75f;
    } else if (status.invalid_count > 0) {
        status.imputation_factor = 0.85f;
    }

    return status;
}

bool DataQualityMonitor::should_force_return_home(
    const DataQualityStatus& status, float battery_level) const {
    return status.emergency_return || battery_level < 15.0f;
}

std::uint8_t DataQualityMonitor::min_missing_for_emergency() const {
    return min_missing_for_emergency_;
}

}  // namespace mili

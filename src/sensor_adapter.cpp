#include "mili/sensor_adapter.hpp"

#include <algorithm>

namespace mili {

SensorAdapter::SensorAdapter(std::uint64_t stale_threshold_ms)
    : stale_threshold_ms_(stale_threshold_ms) {}

void SensorAdapter::ingest_product1(const Product1Data& data, std::uint64_t now_ms) {
    product1_.update(data, now_ms);
}

void SensorAdapter::ingest_product2(const Product2Data& data, std::uint64_t now_ms) {
    product2_.update(data, now_ms);
}

void SensorAdapter::ingest_environmental(const EnvironmentalData& data, std::uint64_t now_ms) {
    if (!data.valid) {
        return;
    }
    environmental_ = data;
    environmental_.timestamp_ms = now_ms;
}

void SensorAdapter::ingest_operator(const OperatorCommand& command, std::uint64_t now_ms) {
    operator_handler_.update(command, now_ms);
}

float SensorAdapter::clamp_battery(float value) {
    return std::max(0.0f, std::min(100.0f, value));
}

SensorInput SensorAdapter::to_sensor_input(std::uint64_t now_ms) const {
    SensorInput input{};
    input.mission_priority = operator_handler_.resolve_priority(default_priority_);

    if (product1_.has_valid_data() && !product1_.is_stale(now_ms, stale_threshold_ms_)) {
        input.battery_level = clamp_battery(product1_.latest_valid().battery_level_percent);
    } else if (product1_.has_valid_data()) {
        input.battery_level = clamp_battery(product1_.latest_valid().battery_level_percent);
    }

    if (product2_.has_valid_data() && !product2_.is_stale(now_ms, stale_threshold_ms_)) {
        input.distance_to_home_km = std::max(0.0f, product2_.latest_valid().distance_to_home_km);
    } else if (product2_.has_valid_data()) {
        input.distance_to_home_km = std::max(0.0f, product2_.latest_valid().distance_to_home_km);
    }

    if (environmental_.valid && now_ms <= environmental_.timestamp_ms + stale_threshold_ms_) {
        input.threat_level = std::max(0.0f, std::min(1.0f, environmental_.threat_level));
        input.comm_strength = std::max(0.0f, std::min(1.0f, environmental_.comm_strength));
        input.target_visibility = std::max(0.0f, std::min(1.0f, environmental_.target_visibility));
        input.wind_speed_mps = std::max(0.0f, environmental_.wind_speed_mps);
        input.weather = environmental_.weather;
        input.time_of_day = environmental_.time_of_day;
    } else if (environmental_.valid) {
        input.threat_level = std::max(0.0f, std::min(1.0f, environmental_.threat_level));
        input.comm_strength = std::max(0.0f, std::min(1.0f, environmental_.comm_strength));
        input.target_visibility = std::max(0.0f, std::min(1.0f, environmental_.target_visibility));
        input.wind_speed_mps = std::max(0.0f, environmental_.wind_speed_mps);
        input.weather = environmental_.weather;
        input.time_of_day = environmental_.time_of_day;
    }

    return input;
}

bool SensorAdapter::product1_is_stale(std::uint64_t now_ms) const {
    return product1_.has_valid_data() && product1_.is_stale(now_ms, stale_threshold_ms_);
}

bool SensorAdapter::product2_is_stale(std::uint64_t now_ms) const {
    return product2_.has_valid_data() && product2_.is_stale(now_ms, stale_threshold_ms_);
}

bool SensorAdapter::environmental_is_stale(std::uint64_t now_ms) const {
    if (!environmental_.valid) {
        return false;
    }
    return now_ms > environmental_.timestamp_ms + stale_threshold_ms_;
}

EnvironmentalData SensorAdapter::last_environmental() const {
    return environmental_;
}

OperatorCommand SensorAdapter::last_operator() const {
    return operator_handler_.latest();
}

}  // namespace mili

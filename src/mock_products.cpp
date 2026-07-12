#include "mili/mock_products.hpp"

namespace mili {

Product1Data MockProduct1::sample(std::uint64_t timestamp_ms, float battery_level) const {
    Product1Data data{};
    data.battery_level_percent = battery_level;
    data.power_draw_watts = 120.0f;
    data.low_power_warning = battery_level < 20.0f;
    data.valid = true;
    data.timestamp_ms = timestamp_ms;
    return data;
}

Product1Data MockProduct1::invalid_sample() const {
    Product1Data data{};
    data.valid = false;
    return data;
}

Product2Data MockProduct2::sample(std::uint64_t timestamp_ms, float distance_km) const {
    Product2Data data{};
    data.position_x_km = distance_km * 0.6f;
    data.position_y_km = distance_km * 0.8f;
    data.distance_to_home_km = distance_km;
    data.velocity_mps = 12.0f;
    data.valid = true;
    data.timestamp_ms = timestamp_ms;
    return data;
}

Product2Data MockProduct2::invalid_sample() const {
    Product2Data data{};
    data.valid = false;
    return data;
}

EnvironmentalData mock_environmental(std::uint64_t timestamp_ms) {
    EnvironmentalData data{};
    data.threat_level = 0.35f;
    data.comm_strength = 0.85f;
    data.target_visibility = 0.75f;
    data.wind_speed_mps = 4.0f;
    data.weather = Weather::Clear;
    data.time_of_day = TimeOfDay::Day;
    data.valid = true;
    data.timestamp_ms = timestamp_ms;
    return data;
}

}  // namespace mili

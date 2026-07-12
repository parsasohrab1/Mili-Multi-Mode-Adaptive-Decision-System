#pragma once

#include "mili/types.hpp"

#include <cstdint>

namespace mili {

struct Product1Data {
    float battery_level_percent = 100.0f;
    float power_draw_watts = 0.0f;
    float remaining_flight_time_min = 0.0f;
    float power_budget_watts = 0.0f;
    bool low_power_warning = false;
    bool valid = false;
    std::uint8_t schema_version = 0;
    std::uint16_t sequence = 0;
    std::uint64_t source_timestamp_ms = 0;
    std::uint64_t timestamp_ms = 0;
};

struct Product2Data {
    float position_x_km = 0.0f;
    float position_y_km = 0.0f;
    float distance_to_home_km = 0.0f;
    float velocity_mps = 0.0f;
    float heading_deg = 0.0f;
    float altitude_m = 0.0f;
    float vio_confidence = 0.0f;
    bool valid = false;
    std::uint8_t schema_version = 0;
    std::uint16_t sequence = 0;
    std::uint64_t source_timestamp_ms = 0;
    std::uint64_t timestamp_ms = 0;
};

struct EnvironmentalData {
    float threat_level = 0.0f;
    float comm_strength = 1.0f;
    float target_visibility = 1.0f;
    float wind_speed_mps = 0.0f;
    Weather weather = Weather::Clear;
    TimeOfDay time_of_day = TimeOfDay::Day;
    bool valid = false;
    std::uint64_t timestamp_ms = 0;
};

struct OperatorCommand {
    bool has_priority_override = false;
    MissionPriority priority_override = MissionPriority::Medium;
    bool valid = false;
    std::uint64_t timestamp_ms = 0;
};

constexpr std::uint64_t kDefaultStaleThresholdMs = 2000;

}  // namespace mili

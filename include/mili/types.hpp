#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace mili {

enum class OperationalMode : std::uint8_t {
    Reconnaissance = 0,
    Surveillance,
    Pursuit,
    Loiter,
    ReturnHome,
    Engagement,
    Count
};

enum class MissionPriority : std::uint8_t {
    Critical = 0,
    High,
    Medium,
    Low
};

enum class Weather : std::uint8_t {
    Clear = 0,
    Cloudy,
    Rain,
    Fog
};

enum class TimeOfDay : std::uint8_t {
    Day = 0,
    Dusk,
    Night
};

struct SensorInput {
    float battery_level = 100.0f;       // 0-100 %
    float threat_level = 0.0f;        // 0-1
    float comm_strength = 1.0f;         // 0-1
    float target_visibility = 1.0f;     // 0-1
    MissionPriority mission_priority = MissionPriority::Medium;
    float distance_to_home_km = 0.0f;
    Weather weather = Weather::Clear;
    TimeOfDay time_of_day = TimeOfDay::Day;
    float wind_speed_mps = 0.0f;
};

struct ModeScore {
    OperationalMode mode;
    float score;
};

struct ModeParameters {
    float speed_factor = 0.5f;
    float altitude_factor = 0.5f;
    float sensor_aggressiveness = 0.5f;
    float engagement_readiness = 0.0f;
    float loiter_radius_km = 0.5f;
    float return_urgency = 0.0f;
};

struct DecisionResult {
    OperationalMode selected_mode;
    float selected_score;
    float confidence;
    ModeParameters control_parameters{};
    std::array<ModeScore, static_cast<std::size_t>(OperationalMode::Count)> all_scores{};
    bool mode_changed;
    bool low_confidence = false;
    bool degraded_inputs = false;
    bool emergency_forced = false;
    std::uint64_t timestamp_ms;
};

struct DecisionLogEntry {
    std::uint64_t timestamp_ms;
    SensorInput input;
    DecisionResult result;
};

constexpr std::size_t kModeCount = static_cast<std::size_t>(OperationalMode::Count);
constexpr std::size_t kMaxLogEntries = 1000;
constexpr float kHysteresisSeconds = 5.0f;
constexpr float kUpdateRateHz = 10.0f;
constexpr float kMaxDistanceKm = 20.0f;
constexpr float kMaxWindSpeedMps = 15.0f;

const char* to_string(OperationalMode mode);
const char* to_string(MissionPriority priority);
const char* to_string(Weather weather);
const char* to_string(TimeOfDay time_of_day);

OperationalMode mode_from_string(const char* value);
MissionPriority priority_from_string(const char* value);
Weather weather_from_string(const char* value);
TimeOfDay time_of_day_from_string(const char* value);

}  // namespace mili

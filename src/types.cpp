#include "mili/types.hpp"

#include <string>

namespace mili {

const char* to_string(OperationalMode mode) {
    switch (mode) {
        case OperationalMode::Reconnaissance: return "reconnaissance";
        case OperationalMode::Surveillance: return "surveillance";
        case OperationalMode::Pursuit: return "pursuit";
        case OperationalMode::Loiter: return "loiter";
        case OperationalMode::ReturnHome: return "return_home";
        case OperationalMode::Engagement: return "engagement";
        default: return "unknown";
    }
}

const char* to_string(MissionPriority priority) {
    switch (priority) {
        case MissionPriority::Critical: return "critical";
        case MissionPriority::High: return "high";
        case MissionPriority::Medium: return "medium";
        case MissionPriority::Low: return "low";
        default: return "unknown";
    }
}

const char* to_string(Weather weather) {
    switch (weather) {
        case Weather::Clear: return "clear";
        case Weather::Cloudy: return "cloudy";
        case Weather::Rain: return "rain";
        case Weather::Fog: return "fog";
        default: return "unknown";
    }
}

const char* to_string(TimeOfDay time_of_day) {
    switch (time_of_day) {
        case TimeOfDay::Day: return "day";
        case TimeOfDay::Dusk: return "dusk";
        case TimeOfDay::Night: return "night";
        default: return "unknown";
    }
}

OperationalMode mode_from_string(const char* value) {
    if (!value) return OperationalMode::Reconnaissance;
    const std::string text(value);
    if (text == "reconnaissance") return OperationalMode::Reconnaissance;
    if (text == "surveillance") return OperationalMode::Surveillance;
    if (text == "pursuit") return OperationalMode::Pursuit;
    if (text == "loiter") return OperationalMode::Loiter;
    if (text == "return_home") return OperationalMode::ReturnHome;
    if (text == "engagement") return OperationalMode::Engagement;
    return OperationalMode::Reconnaissance;
}

MissionPriority priority_from_string(const char* value) {
    if (!value) return MissionPriority::Medium;
    const std::string text(value);
    if (text == "critical") return MissionPriority::Critical;
    if (text == "high") return MissionPriority::High;
    if (text == "medium") return MissionPriority::Medium;
    if (text == "low") return MissionPriority::Low;
    return MissionPriority::Medium;
}

Weather weather_from_string(const char* value) {
    if (!value) return Weather::Clear;
    const std::string text(value);
    if (text == "clear") return Weather::Clear;
    if (text == "cloudy") return Weather::Cloudy;
    if (text == "rain") return Weather::Rain;
    if (text == "fog") return Weather::Fog;
    return Weather::Clear;
}

TimeOfDay time_of_day_from_string(const char* value) {
    if (!value) return TimeOfDay::Day;
    const std::string text(value);
    if (text == "day") return TimeOfDay::Day;
    if (text == "dusk") return TimeOfDay::Dusk;
    if (text == "night") return TimeOfDay::Night;
    return TimeOfDay::Day;
}

}  // namespace mili

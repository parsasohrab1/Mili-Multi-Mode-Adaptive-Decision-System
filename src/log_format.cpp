#include "mili/log_format.hpp"

#include <cstdio>
#include <cstring>

namespace mili::logfmt {

const char* csv_header() {
    return "timestamp_ms,battery,threat,comm,visibility,priority,distance_km,mode,score,confidence,"
           "mode_changed,low_confidence,degraded_inputs,emergency_forced\n";
}

std::size_t format_csv_line(char* buffer, std::size_t capacity, const DecisionLogEntry& entry) {
    if (buffer == nullptr || capacity == 0) {
        return 0;
    }
    const auto& input = entry.input;
    const auto& result = entry.result;
    const int written = std::snprintf(
        buffer,
        capacity,
        "%llu,%.2f,%.4f,%.4f,%.4f,%s,%.2f,%s,%.4f,%.4f,%d,%d,%d,%d\n",
        static_cast<unsigned long long>(entry.timestamp_ms),
        input.battery_level,
        input.threat_level,
        input.comm_strength,
        input.target_visibility,
        to_string(input.mission_priority),
        input.distance_to_home_km,
        to_string(result.selected_mode),
        result.selected_score,
        result.confidence,
        result.mode_changed ? 1 : 0,
        result.low_confidence ? 1 : 0,
        result.degraded_inputs ? 1 : 0,
        result.emergency_forced ? 1 : 0);
    if (written < 0) {
        return 0;
    }
    return static_cast<std::size_t>(written);
}

std::size_t format_json_entry(char* buffer, std::size_t capacity, const DecisionLogEntry& entry, bool first) {
    if (buffer == nullptr || capacity == 0) {
        return 0;
    }
    const char* prefix = first ? "" : ",";
    const int written = std::snprintf(
        buffer,
        capacity,
        "%s{\"timestamp_ms\":%llu,\"battery\":%.2f,\"threat\":%.4f,\"comm\":%.4f,"
        "\"visibility\":%.4f,\"priority\":\"%s\",\"distance_km\":%.2f,"
        "\"mode\":\"%s\",\"score\":%.4f,\"confidence\":%.4f,"
        "\"mode_changed\":%s,\"low_confidence\":%s,\"degraded_inputs\":%s,\"emergency_forced\":%s}",
        prefix,
        static_cast<unsigned long long>(entry.timestamp_ms),
        entry.input.battery_level,
        entry.input.threat_level,
        entry.input.comm_strength,
        entry.input.target_visibility,
        to_string(entry.input.mission_priority),
        entry.input.distance_to_home_km,
        to_string(entry.result.selected_mode),
        entry.result.selected_score,
        entry.result.confidence,
        entry.result.mode_changed ? "true" : "false",
        entry.result.low_confidence ? "true" : "false",
        entry.result.degraded_inputs ? "true" : "false",
        entry.result.emergency_forced ? "true" : "false");
    if (written < 0) {
        return 0;
    }
    return static_cast<std::size_t>(written);
}

}  // namespace mili::logfmt

#include "mili/weight_manager.hpp"

#include "mili/config_version.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace mili {

namespace {

bool iequals(const std::string& left, const char* right) {
    const std::string r(right);
    if (left.size() != r.size()) {
        return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(left[i]))
            != std::tolower(static_cast<unsigned char>(r[i]))) {
            return false;
        }
    }
    return true;
}

bool find_float_after_key(const std::string& json, const std::string& key, float& out_value) {
    const std::string needle = "\"" + key + "\"";
    const auto key_pos = json.find(needle);
    if (key_pos == std::string::npos) {
        return false;
    }

    const auto colon_pos = json.find(':', key_pos + needle.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    std::size_t start = colon_pos + 1;
    while (start < json.size() && std::isspace(static_cast<unsigned char>(json[start]))) {
        ++start;
    }

    std::size_t end = start;
    while (end < json.size()
           && (std::isdigit(static_cast<unsigned char>(json[end]))
               || json[end] == '.' || json[end] == '-' || json[end] == '+' || json[end] == 'e'
               || json[end] == 'E')) {
        ++end;
    }

    if (end == start) {
        return false;
    }

    try {
        out_value = std::stof(json.substr(start, end - start));
        return true;
    } catch (...) {
        return false;
    }
}

bool find_string_after_key(const std::string& json, const std::string& key, std::string& out_value) {
    const std::string needle = "\"" + key + "\"";
    const auto key_pos = json.find(needle);
    if (key_pos == std::string::npos) {
        return false;
    }

    const auto colon_pos = json.find(':', key_pos + needle.size());
    if (colon_pos == std::string::npos) {
        return false;
    }

    std::size_t start = colon_pos + 1;
    while (start < json.size() && std::isspace(static_cast<unsigned char>(json[start]))) {
        ++start;
    }
    if (start >= json.size() || json[start] != '"') {
        return false;
    }
    ++start;

    const auto end = json.find('"', start);
    if (end == std::string::npos) {
        return false;
    }

    out_value = json.substr(start, end - start);
    return true;
}

std::string extract_object(const std::string& json, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    const auto key_pos = json.find(needle);
    if (key_pos == std::string::npos) {
        return {};
    }

    const auto open_pos = json.find('{', key_pos);
    if (open_pos == std::string::npos) {
        return {};
    }

    int depth = 0;
    for (std::size_t i = open_pos; i < json.size(); ++i) {
        if (json[i] == '{') {
            ++depth;
        } else if (json[i] == '}') {
            --depth;
            if (depth == 0) {
                return json.substr(open_pos, i - open_pos + 1);
            }
        }
    }

    return {};
}

bool parse_four_term_block(const std::string& block, const char* keys[4], FourTermModeWeights& out) {
    bool found_any = false;
    for (int i = 0; i < 4; ++i) {
        float value = 0.0f;
        if (find_float_after_key(block, keys[i], value)) {
            out.terms[i] = value;
            found_any = true;
        }
    }
    return found_any;
}

}  // namespace

WeightManager::WeightManager()
    : active_(make_default_weight_config())
    , defaults_(make_default_weight_config())
    , reload_count_(0)
    , using_defaults_(true) {}

bool WeightManager::load_from_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        restore_defaults();
        return false;
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    parse_buffer_ = stream.str();
    last_path_ = path;

    const bool ok = apply_json(parse_buffer_);
    if (!ok) {
        restore_defaults();
        return false;
    }

    using_defaults_ = false;
    ++reload_count_;
    return true;
}

bool WeightManager::load_from_string(const std::string& json) {
    parse_buffer_ = json;
    const bool ok = apply_json(parse_buffer_);
    if (!ok) {
        restore_defaults();
        return false;
    }

    using_defaults_ = false;
    ++reload_count_;
    return true;
}

bool WeightManager::reload() {
    if (last_path_.empty()) {
        restore_defaults();
        return false;
    }
    return load_from_file(last_path_);
}

const WeightConfig& WeightManager::config() const {
    return active_;
}

WeightConfig& WeightManager::config() {
    using_defaults_ = false;
    return active_;
}

bool WeightManager::set_mode_factor(OperationalMode mode, const char* factor_name, float value) {
    if (factor_name == nullptr) {
        return false;
    }

    using_defaults_ = false;

    switch (mode) {
        case OperationalMode::Reconnaissance: {
            const char* keys[4] = {"battery", "threat_inverse", "comm_strength", "distance_inverse"};
            for (int i = 0; i < 4; ++i) {
                if (iequals(factor_name, keys[i])) {
                    active_.reconnaissance.terms[i] = value;
                    return true;
                }
            }
            return false;
        }
        case OperationalMode::Surveillance: {
            const char* keys[4] = {"target_visibility", "threat_inverse", "battery", "wind_inverse"};
            for (int i = 0; i < 4; ++i) {
                if (iequals(factor_name, keys[i])) {
                    active_.surveillance.terms[i] = value;
                    return true;
                }
            }
            return false;
        }
        case OperationalMode::Pursuit: {
            const char* keys[4] = {"target_visibility", "battery_threshold", "threat_inverse", "comm_threshold"};
            for (int i = 0; i < 4; ++i) {
                if (iequals(factor_name, keys[i])) {
                    active_.pursuit.terms[i] = value;
                    return true;
                }
            }
            return false;
        }
        case OperationalMode::Loiter: {
            const char* keys[4] = {"comm_inverse", "threat_threshold", "visibility_inverse", "battery"};
            for (int i = 0; i < 4; ++i) {
                if (iequals(factor_name, keys[i])) {
                    active_.loiter.terms[i] = value;
                    return true;
                }
            }
            return false;
        }
        case OperationalMode::ReturnHome: {
            const char* keys[4] = {"battery_inverse", "threat", "distance_threshold", "comm_inverse"};
            for (int i = 0; i < 4; ++i) {
                if (iequals(factor_name, keys[i])) {
                    active_.return_home.terms[i] = value;
                    return true;
                }
            }
            return false;
        }
        case OperationalMode::Engagement:
            if (iequals(factor_name, "target_visibility")) {
                active_.engagement.target_visibility = value;
                return true;
            }
            if (iequals(factor_name, "threat_threshold")) {
                active_.engagement.threat_threshold = value;
                return true;
            }
            if (iequals(factor_name, "battery_threshold")) {
                active_.engagement.battery_threshold = value;
                return true;
            }
            if (iequals(factor_name, "priority_boost")) {
                active_.engagement.priority_boost = value;
                return true;
            }
            if (iequals(factor_name, "distance_penalty")) {
                active_.engagement.distance_penalty = value;
                return true;
            }
            return false;
        default:
            return false;
    }
}

bool WeightManager::set_priority_multiplier(
    MissionPriority priority, OperationalMode mode, float multiplier) {
    PriorityMultiplierSet* set = nullptr;
    if (priority == MissionPriority::Critical) {
        set = &active_.critical;
    } else if (priority == MissionPriority::Low) {
        set = &active_.low;
    } else {
        return false;
    }

    using_defaults_ = false;
    switch (mode) {
        case OperationalMode::Engagement: set->engagement = multiplier; return true;
        case OperationalMode::Surveillance: set->surveillance = multiplier; return true;
        case OperationalMode::Loiter: set->loiter = multiplier; return true;
        case OperationalMode::ReturnHome: set->return_home = multiplier; return true;
        case OperationalMode::Reconnaissance: set->reconnaissance = multiplier; return true;
        default: return false;
    }
}

bool WeightManager::set_environmental_multiplier(
    const char* condition, OperationalMode mode, float multiplier) {
    if (condition == nullptr) {
        return false;
    }

    using_defaults_ = false;

    if (iequals(condition, "rain")) {
        if (mode == OperationalMode::Surveillance) {
            active_.environmental.rain_surveillance = multiplier;
            return true;
        }
        if (mode == OperationalMode::Pursuit) {
            active_.environmental.rain_pursuit = multiplier;
            return true;
        }
        return false;
    }

    if (iequals(condition, "fog")) {
        if (mode == OperationalMode::Reconnaissance) {
            active_.environmental.fog_reconnaissance = multiplier;
            return true;
        }
        if (mode == OperationalMode::Surveillance) {
            active_.environmental.fog_surveillance = multiplier;
            return true;
        }
        return false;
    }

    if (iequals(condition, "night")) {
        if (mode == OperationalMode::Reconnaissance) {
            active_.environmental.night_reconnaissance = multiplier;
            return true;
        }
        if (mode == OperationalMode::Surveillance) {
            active_.environmental.night_surveillance = multiplier;
            return true;
        }
        return false;
    }

    return false;
}

const std::string& WeightManager::last_loaded_path() const {
    return last_path_;
}

bool WeightManager::using_defaults() const {
    return using_defaults_;
}

std::size_t WeightManager::reload_count() const {
    return reload_count_;
}

std::size_t WeightManager::buffer_capacity() const {
    return parse_buffer_.capacity();
}

WeightConfig WeightManager::default_config() {
    return make_default_weight_config();
}

bool WeightManager::apply_json(const std::string& json) {
    if (json.find('{') == std::string::npos) {
        return false;
    }

    std::string version;
    if (!find_string_after_key(json, "version", version)
        || !is_compatible_weight_config(version)) {
        return false;
    }

    WeightConfig parsed = defaults_;

    const std::string modes = extract_object(json, "modes");
    if (modes.empty()) {
        return false;
    }

    const std::string recon = extract_object(modes, "reconnaissance");
    const std::string surv = extract_object(modes, "surveillance");
    const std::string pursuit = extract_object(modes, "pursuit");
    const std::string loiter = extract_object(modes, "loiter");
    const std::string ret = extract_object(modes, "return_home");
    const std::string engage = extract_object(modes, "engagement");

    const char* recon_keys[4] = {"battery", "threat_inverse", "comm_strength", "distance_inverse"};
    const char* surv_keys[4] = {"target_visibility", "threat_inverse", "battery", "wind_inverse"};
    const char* pursuit_keys[4] = {"target_visibility", "battery_threshold", "threat_inverse", "comm_threshold"};
    const char* loiter_keys[4] = {"comm_inverse", "threat_threshold", "visibility_inverse", "battery"};
    const char* ret_keys[4] = {"battery_inverse", "threat", "distance_threshold", "comm_inverse"};

    if (!parse_four_term_block(recon, recon_keys, parsed.reconnaissance)
        || !parse_four_term_block(surv, surv_keys, parsed.surveillance)
        || !parse_four_term_block(pursuit, pursuit_keys, parsed.pursuit)
        || !parse_four_term_block(loiter, loiter_keys, parsed.loiter)
        || !parse_four_term_block(ret, ret_keys, parsed.return_home)) {
        return false;
    }

    if (!find_float_after_key(engage, "target_visibility", parsed.engagement.target_visibility)
        || !find_float_after_key(engage, "threat_threshold", parsed.engagement.threat_threshold)
        || !find_float_after_key(engage, "battery_threshold", parsed.engagement.battery_threshold)
        || !find_float_after_key(engage, "priority_boost", parsed.engagement.priority_boost)
        || !find_float_after_key(engage, "distance_penalty", parsed.engagement.distance_penalty)) {
        return false;
    }

    const std::string priority = extract_object(json, "priority_multipliers");
    const std::string critical = extract_object(priority, "critical");
    const std::string low = extract_object(priority, "low");

    find_float_after_key(critical, "engagement", parsed.critical.engagement);
    find_float_after_key(critical, "surveillance", parsed.critical.surveillance);
    find_float_after_key(low, "loiter", parsed.low.loiter);
    find_float_after_key(low, "return_home", parsed.low.return_home);
    find_float_after_key(low, "reconnaissance", parsed.low.reconnaissance);

    const std::string environmental = extract_object(json, "environmental");
    const std::string rain = extract_object(environmental, "rain");
    const std::string fog = extract_object(environmental, "fog");
    const std::string night = extract_object(environmental, "night");

    find_float_after_key(rain, "surveillance", parsed.environmental.rain_surveillance);
    find_float_after_key(rain, "pursuit", parsed.environmental.rain_pursuit);
    find_float_after_key(fog, "reconnaissance", parsed.environmental.fog_reconnaissance);
    find_float_after_key(fog, "surveillance", parsed.environmental.fog_surveillance);
    find_float_after_key(night, "reconnaissance", parsed.environmental.night_reconnaissance);
    find_float_after_key(night, "surveillance", parsed.environmental.night_surveillance);

    find_float_after_key(json, "hysteresis_seconds", parsed.hysteresis_seconds);
    find_float_after_key(json, "update_rate_hz", parsed.update_rate_hz);

    float max_entries = static_cast<float>(parsed.max_log_entries);
    if (find_float_after_key(json, "max_decision_log_entries", max_entries)) {
        parsed.max_log_entries = static_cast<std::size_t>(max_entries);
    }

    active_ = parsed;
    return true;
}

void WeightManager::restore_defaults() {
    active_ = defaults_;
    using_defaults_ = true;
}

}  // namespace mili

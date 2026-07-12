#pragma once

#include "mili/weight_config.hpp"

#include <string>

namespace mili {

class WeightManager {
public:
    WeightManager();

    bool load_from_file(const std::string& path);
    bool load_from_string(const std::string& json);
    bool reload();

    const WeightConfig& config() const;
    WeightConfig& config();

    bool set_mode_factor(OperationalMode mode, const char* factor_name, float value);
    bool set_priority_multiplier(MissionPriority priority, OperationalMode mode, float multiplier);
    bool set_environmental_multiplier(const char* condition, OperationalMode mode, float multiplier);

    const std::string& last_loaded_path() const;
    bool using_defaults() const;
    std::size_t reload_count() const;
    std::size_t buffer_capacity() const;

    static WeightConfig default_config();

private:
    WeightConfig active_;
    WeightConfig defaults_;
    std::string last_path_;
    std::string parse_buffer_;
    std::size_t reload_count_;
    bool using_defaults_;

    bool apply_json(const std::string& json);
    void restore_defaults();
};

}  // namespace mili

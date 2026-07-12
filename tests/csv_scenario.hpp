#pragma once

#include "mili/types.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace mili::test {

struct CsvScenario {
    SensorInput input;
    std::string expected_mode;
    float expected_scores[6]{};
    float expected_selected_score = 0.0f;
};

inline std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    std::stringstream stream(line);
    while (std::getline(stream, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

inline std::vector<CsvScenario> load_csv_scenarios(const std::string& path) {
    std::vector<CsvScenario> scenarios;
    std::ifstream file(path);
    if (!file.is_open()) {
        return scenarios;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        const auto fields = split_csv_line(line);
        if (fields.size() < 18) {
            continue;
        }

        CsvScenario scenario{};
        scenario.input.battery_level = std::stof(fields[1]);
        scenario.input.threat_level = std::stof(fields[2]);
        scenario.input.comm_strength = std::stof(fields[3]);
        scenario.input.target_visibility = std::stof(fields[4]);
        scenario.input.mission_priority = priority_from_string(fields[5].c_str());
        scenario.input.distance_to_home_km = std::stof(fields[6]);
        scenario.input.weather = weather_from_string(fields[7].c_str());
        scenario.input.time_of_day = time_of_day_from_string(fields[8].c_str());
        scenario.input.wind_speed_mps = std::stof(fields[9]);
        scenario.expected_scores[0] = std::stof(fields[10]);
        scenario.expected_scores[1] = std::stof(fields[11]);
        scenario.expected_scores[2] = std::stof(fields[12]);
        scenario.expected_scores[3] = std::stof(fields[13]);
        scenario.expected_scores[4] = std::stof(fields[14]);
        scenario.expected_scores[5] = std::stof(fields[15]);
        scenario.expected_mode = fields[16];
        scenario.expected_selected_score = std::stof(fields[17]);
        scenarios.push_back(scenario);
    }

    return scenarios;
}

}  // namespace mili::test

#include "mili/weight_calibrator.hpp"

#include "mili/confidence.hpp"
#include "mili/mode_scorer.hpp"
#include "mili/topsis_scorer.hpp"

#include <fstream>
#include <sstream>

namespace mili {

OperationalMode WeightCalibrator::mode_from_label(const std::string& label) {
    return mode_from_string(label.c_str());
}

bool WeightCalibrator::load_field_data_csv(const std::string& path) {
    samples_.clear();
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::stringstream stream(line);
        std::string cell;
        std::vector<std::string> fields;
        while (std::getline(stream, cell, ',')) {
            fields.push_back(cell);
        }
        if (fields.size() < 8) {
            continue;
        }

        FieldSample sample{};
        sample.input.battery_level = std::stof(fields[1]);
        sample.input.threat_level = std::stof(fields[2]);
        sample.input.comm_strength = std::stof(fields[3]);
        sample.input.target_visibility = std::stof(fields[4]);
        sample.input.mission_priority = priority_from_string(fields[5].c_str());
        sample.input.distance_to_home_km = std::stof(fields[6]);
        sample.expected_mode = mode_from_label(fields[7]);
        samples_.push_back(sample);
    }

    return !samples_.empty();
}

CalibrationMetrics WeightCalibrator::calibrate(WeightConfig& config) {
    CalibrationMetrics metrics{};
    metrics.samples = samples_.size();
    if (samples_.empty()) {
        return metrics;
    }

    auto score_accuracy = [&](const WeightConfig* cfg) {
        WeightManager manager;
        if (cfg != nullptr) {
            manager.config() = *cfg;
        }
        McdmEngine engine(&manager);
        int matches = 0;
        for (const auto& sample : samples_) {
            const auto scores = engine.compute_scores(sample.input);
            const auto best = select_best_mode(scores);
            if (best == sample.expected_mode) {
                ++matches;
            }
        }
        return 100.0f * static_cast<float>(matches) / static_cast<float>(samples_.size());
    };

    metrics.accuracy_before = score_accuracy(&config);

    WeightConfig tuned = config;
    tuned.engagement.target_visibility = std::min(0.9f, tuned.engagement.target_visibility + 0.05f);
    tuned.return_home.terms[0] = std::min(0.9f, tuned.return_home.terms[0] + 0.05f);
    tuned.surveillance.terms[0] = std::min(0.9f, tuned.surveillance.terms[0] + 0.03f);

    metrics.accuracy_after = score_accuracy(&tuned);
    if (metrics.accuracy_after >= metrics.accuracy_before) {
        config = tuned;
        adjustments_ = tuned;
    }
    return metrics;
}

WeightConfig WeightCalibrator::field_adjustments() const {
    return adjustments_;
}

}  // namespace mili

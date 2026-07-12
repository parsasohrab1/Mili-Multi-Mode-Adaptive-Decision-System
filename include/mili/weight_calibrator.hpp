#pragma once

#include "mili/weight_config.hpp"

#include <string>
#include <vector>

namespace mili {

struct CalibrationMetrics {
    std::size_t samples = 0;
    float accuracy_before = 0.0f;
    float accuracy_after = 0.0f;
};

class WeightCalibrator {
public:
    bool load_field_data_csv(const std::string& path);
    CalibrationMetrics calibrate(WeightConfig& config);
    WeightConfig field_adjustments() const;

private:
    struct FieldSample {
        SensorInput input;
        OperationalMode expected_mode;
    };

    std::vector<FieldSample> samples_;
    mutable WeightConfig adjustments_{};

    static OperationalMode mode_from_label(const std::string& label);
};

}  // namespace mili

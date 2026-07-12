#pragma once

#include "mili/types.hpp"

namespace mili {

class SensorNoiseModel {
public:
    static constexpr float kBaseErrorRate = 0.07f;

    explicit SensorNoiseModel(float error_rate = kBaseErrorRate);

    SensorInput apply(const SensorInput& input, float confidence, std::uint32_t seed) const;

private:
    float error_rate_;
};

}  // namespace mili

#include "mili/sensor_noise_model.hpp"

#include <cmath>

namespace mili {

namespace {

float pseudo_random(std::uint32_t seed) {
    seed = (seed * 1664525u) + 1013904223u;
    return static_cast<float>(seed & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

float perturb(float value, float magnitude, std::uint32_t& seed) {
    const auto rnd = pseudo_random(seed++);
    return std::max(0.0f, std::min(1.0f, value + (rnd - 0.5f) * 2.0f * magnitude));
}

}  // namespace

SensorNoiseModel::SensorNoiseModel(float error_rate)
    : error_rate_(error_rate) {}

SensorInput SensorNoiseModel::apply(const SensorInput& input, float confidence, std::uint32_t seed) const {
    const float effective_rate = error_rate_ * (1.0f - confidence);
    if (pseudo_random(seed) > effective_rate) {
        return input;
    }

    SensorInput noisy = input;
    const float magnitude = 0.15f * (1.0f - confidence);
    noisy.threat_level = perturb(input.threat_level, magnitude, seed);
    noisy.comm_strength = perturb(input.comm_strength, magnitude, seed);
    noisy.target_visibility = perturb(input.target_visibility, magnitude, seed);
    noisy.battery_level = std::max(
        0.0f,
        std::min(100.0f, input.battery_level + (pseudo_random(seed) - 0.5f) * 10.0f));
    return noisy;
}

}  // namespace mili

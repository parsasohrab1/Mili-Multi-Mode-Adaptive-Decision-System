#include "mili/confidence.hpp"

#include <algorithm>
#include <cmath>

namespace mili {

OperationalMode select_best_mode(const std::array<float, kModeCount>& scores) {
    auto best_idx = static_cast<std::size_t>(OperationalMode::Reconnaissance);
    for (std::size_t i = 1; i < kModeCount; ++i) {
        if (scores[i] > scores[best_idx]) {
            best_idx = i;
        }
    }
    return static_cast<OperationalMode>(best_idx);
}

float compute_confidence(const std::array<float, kModeCount>& scores) {
    std::array<float, kModeCount> sorted = scores;
    std::sort(sorted.begin(), sorted.end(), std::greater<float>());

    if (sorted[0] <= 0.0f) {
        return 0.3f;
    }

    const float confidence = 0.5f + 0.5f * (1.0f - (sorted[1] / sorted[0]));
    return std::max(0.3f, std::min(0.99f, confidence));
}

}  // namespace mili

#pragma once

#include "mili/types.hpp"

#include <array>

namespace mili {

float compute_confidence(const std::array<float, kModeCount>& scores);
OperationalMode select_best_mode(const std::array<float, kModeCount>& scores);

}  // namespace mili

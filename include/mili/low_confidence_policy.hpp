#pragma once

#include "mili/types.hpp"

namespace mili {

constexpr float kLowConfidenceThreshold = 0.5f;

OperationalMode apply_low_confidence_policy(OperationalMode candidate, float confidence);
void apply_low_confidence_parameters(ModeParameters& params, float confidence);

}  // namespace mili

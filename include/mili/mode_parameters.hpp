#pragma once

#include "mili/types.hpp"

namespace mili {

class ModeParameterMapper {
public:
    static ModeParameters for_mode(OperationalMode mode, const SensorInput& input, float mode_score);
};

}  // namespace mili

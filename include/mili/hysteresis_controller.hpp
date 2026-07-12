#pragma once

#include "mili/types.hpp"

namespace mili {

class HysteresisController {
public:
    explicit HysteresisController(float hysteresis_seconds = kHysteresisSeconds);

    bool should_switch(OperationalMode current, OperationalMode candidate, std::uint64_t timestamp_ms);

    void reset();
    void set_hysteresis_seconds(float seconds);
    float hysteresis_seconds() const;

private:
    float hysteresis_seconds_;
    OperationalMode pending_mode_;
    std::uint64_t pending_since_ms_;
    bool has_pending_;
};

}  // namespace mili

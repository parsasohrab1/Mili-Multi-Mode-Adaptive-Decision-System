#pragma once

#include "mili/types.hpp"

#include <cstdint>

namespace mili {

enum class FsmState : std::uint8_t {
    Reconnaissance = 0,
    Surveillance,
    Pursuit,
    Loiter,
    ReturnHome,
    Engagement,
    Emergency,
    Count
};

struct TransitionGuard {
    float min_battery = 0.0f;
    float max_threat = 1.0f;
    bool allow_low_confidence = false;
};

struct FsmTransitionResult {
    FsmState next_state;
    bool allowed;
    bool is_emergency;
};

class ModeStateMachine {
public:
    ModeStateMachine();

    FsmState current_state() const;
    OperationalMode current_mode() const;

    FsmTransitionResult evaluate_transition(
        OperationalMode candidate,
        const SensorInput& input,
        float confidence,
        bool quality_emergency) const;

    void apply_state(FsmState state);
    void reset();

    static FsmState mode_to_state(OperationalMode mode);

private:
    FsmState state_;

    static FsmState to_fsm_state(OperationalMode mode);
    static OperationalMode to_operational_mode(FsmState state);
    static TransitionGuard guard_for(FsmState from, FsmState to);
    static bool guard_passes(const TransitionGuard& guard, const SensorInput& input, float confidence);
};

}  // namespace mili

#include "mili/mode_state_machine.hpp"

namespace mili {

ModeStateMachine::ModeStateMachine()
    : state_(FsmState::Reconnaissance) {}

FsmState ModeStateMachine::current_state() const {
    return state_;
}

OperationalMode ModeStateMachine::current_mode() const {
    return to_operational_mode(state_);
}

FsmState ModeStateMachine::mode_to_state(OperationalMode mode) {
    return to_fsm_state(mode);
}

FsmState ModeStateMachine::to_fsm_state(OperationalMode mode) {
    return static_cast<FsmState>(mode);
}

OperationalMode ModeStateMachine::to_operational_mode(FsmState state) {
    if (state == FsmState::Emergency) {
        return OperationalMode::ReturnHome;
    }
    return static_cast<OperationalMode>(state);
}

TransitionGuard ModeStateMachine::guard_for(FsmState from, FsmState to) {
    TransitionGuard guard{};
    if (to == FsmState::Emergency || to == FsmState::ReturnHome) {
        guard.min_battery = 0.0f;
        guard.max_threat = 1.0f;
        guard.allow_low_confidence = true;
        return guard;
    }
    if (to == FsmState::Engagement) {
        guard.min_battery = 30.0f;
        guard.max_threat = 1.0f;
        return guard;
    }
    if (from == FsmState::Pursuit && to == FsmState::Loiter) {
        guard.min_battery = 20.0f;
    }
    return guard;
}

bool ModeStateMachine::guard_passes(
    const TransitionGuard& guard, const SensorInput& input, float confidence) {
    if (input.battery_level < guard.min_battery) {
        return false;
    }
    if (input.threat_level > guard.max_threat) {
        return false;
    }
    if (!guard.allow_low_confidence && confidence < 0.5f) {
        return false;
    }
    return true;
}

FsmTransitionResult ModeStateMachine::evaluate_transition(
    OperationalMode candidate,
    const SensorInput& input,
    float confidence,
    bool quality_emergency) const {
    FsmTransitionResult result{};

    if (quality_emergency || input.battery_level < 15.0f) {
        result.next_state = FsmState::Emergency;
        result.allowed = true;
        result.is_emergency = true;
        return result;
    }

    const auto target = to_fsm_state(candidate);
    const auto guard = guard_for(state_, target);
    result.next_state = target;
    result.allowed = guard_passes(guard, input, confidence);
    result.is_emergency = false;
    return result;
}

void ModeStateMachine::apply_state(FsmState state) {
    state_ = state;
}

void ModeStateMachine::reset() {
    state_ = FsmState::Reconnaissance;
}

}  // namespace mili

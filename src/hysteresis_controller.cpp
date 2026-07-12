#include "mili/hysteresis_controller.hpp"

namespace mili {

HysteresisController::HysteresisController(float hysteresis_seconds)
    : hysteresis_seconds_(hysteresis_seconds)
    , pending_mode_(OperationalMode::Reconnaissance)
    , pending_since_ms_(0)
    , has_pending_(false) {}

bool HysteresisController::should_switch(OperationalMode current, OperationalMode candidate, std::uint64_t timestamp_ms) {
    if (current == candidate) {
        has_pending_ = false;
        return false;
    }

    if (!has_pending_ || pending_mode_ != candidate) {
        pending_mode_ = candidate;
        pending_since_ms_ = timestamp_ms;
        has_pending_ = true;
        return false;
    }

    const auto elapsed_ms = timestamp_ms - pending_since_ms_;
    const auto required_ms = static_cast<std::uint64_t>(hysteresis_seconds_ * 1000.0f);

    if (elapsed_ms >= required_ms) {
        has_pending_ = false;
        return true;
    }

    return false;
}

void HysteresisController::reset() {
    has_pending_ = false;
}

void HysteresisController::set_hysteresis_seconds(float seconds) {
    hysteresis_seconds_ = seconds;
}

float HysteresisController::hysteresis_seconds() const {
    return hysteresis_seconds_;
}

}  // namespace mili

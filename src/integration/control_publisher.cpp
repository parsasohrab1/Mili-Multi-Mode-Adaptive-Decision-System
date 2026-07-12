#include "mili/integration/control_publisher.hpp"

namespace mili {

ControlSystemPublisher::ControlSystemPublisher(CanSendCallback send_frame, void* user_data)
    : send_frame_(send_frame)
    , user_data_(user_data)
    , publish_count_(0) {}

bool ControlSystemPublisher::publish(const DecisionResult& result) {
    last_mode_ = protocol::encode_mode_command(result);
    last_params_ = protocol::encode_control_parameters(result);

    if (send_frame_ == nullptr) {
        return false;
    }

    const bool mode_ok = CanRouter::send_frame(
        send_frame_, user_data_, protocol::kCanIdModeCommand, &last_mode_, sizeof(last_mode_));
    const bool params_ok = CanRouter::send_frame(
        send_frame_, user_data_, protocol::kCanIdControlParams, &last_params_, sizeof(last_params_));

    if (mode_ok && params_ok) {
        ++publish_count_;
    }
    return mode_ok && params_ok;
}

const protocol::ModeCommandPayload& ControlSystemPublisher::last_mode_command() const {
    return last_mode_;
}

const protocol::ControlParametersPayload& ControlSystemPublisher::last_control_params() const {
    return last_params_;
}

std::uint32_t ControlSystemPublisher::publish_count() const {
    return publish_count_;
}

}  // namespace mili

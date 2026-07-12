#pragma once

#include "mili/integration/interfaces.hpp"
#include "mili/platform/can_router.hpp"
#include "mili/protocol/messages.hpp"

#include <cstdint>

namespace mili {

// Publishes mode command + full control parameters to the flight control system.
class ControlSystemPublisher : public IControlPublisher {
public:
    explicit ControlSystemPublisher(CanSendCallback send_frame, void* user_data = nullptr);

    bool publish(const DecisionResult& result) override;

    const protocol::ModeCommandPayload& last_mode_command() const;
    const protocol::ControlParametersPayload& last_control_params() const;

    std::uint32_t publish_count() const;

private:
    CanSendCallback send_frame_;
    void* user_data_;
    protocol::ModeCommandPayload last_mode_{};
    protocol::ControlParametersPayload last_params_{};
    std::uint32_t publish_count_;
};

}  // namespace mili

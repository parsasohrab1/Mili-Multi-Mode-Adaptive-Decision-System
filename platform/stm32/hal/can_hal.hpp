#pragma once

#include "mili/platform/can_router.hpp"

#include <cstddef>
#include <cstdint>

namespace mili::platform {

struct CanFrame {
    std::uint16_t id;
    std::uint8_t data[8];
    std::uint8_t len;
};

using CanHalSendFn = CanSendCallback;
using CanHalReceiveFn = void (*)(const CanFrame& frame, void* user_data);

class CanHal {
public:
    CanHal();

    void set_send_callback(CanHalSendFn send, void* user_data);
    void set_receive_handler(CanHalReceiveFn handler, void* user_data);

    bool send(std::uint16_t can_id, const void* data, std::size_t len);
    void on_rx_isr(const CanFrame& frame);

    std::uint32_t tx_count() const;
    std::uint32_t rx_count() const;

private:
    CanHalSendFn send_;
    void* send_ctx_;
    CanHalReceiveFn receive_;
    void* receive_ctx_;
    std::uint32_t tx_count_;
    std::uint32_t rx_count_;
};

}  // namespace mili::platform

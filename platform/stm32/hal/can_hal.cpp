#include "can_hal.hpp"

#include <cstring>

namespace mili::platform {

CanHal::CanHal()
    : send_(nullptr)
    , send_ctx_(nullptr)
    , receive_(nullptr)
    , receive_ctx_(nullptr)
    , tx_count_(0)
    , rx_count_(0) {}

void CanHal::set_send_callback(CanHalSendFn send, void* user_data) {
    send_ = send;
    send_ctx_ = user_data;
}

void CanHal::set_receive_handler(CanHalReceiveFn handler, void* user_data) {
    receive_ = handler;
    receive_ctx_ = user_data;
}

bool CanHal::send(std::uint16_t can_id, const void* data, std::size_t len) {
    if (send_ == nullptr || data == nullptr || len == 0 || len > 8) {
        return false;
    }
    const bool ok = send_(can_id, data, len, send_ctx_);
    if (ok) {
        ++tx_count_;
    }
    return ok;
}

void CanHal::on_rx_isr(const CanFrame& frame) {
    ++rx_count_;
    if (receive_ != nullptr) {
        receive_(frame, receive_ctx_);
    }
}

std::uint32_t CanHal::tx_count() const {
    return tx_count_;
}

std::uint32_t CanHal::rx_count() const {
    return rx_count_;
}

}  // namespace mili::platform

#if defined(MILI_STM32_FDCAN)
extern "C" void HAL_FDCAN_RxFifo0Callback(void* hfdcan, uint32_t rx_fifo_ix);
#endif

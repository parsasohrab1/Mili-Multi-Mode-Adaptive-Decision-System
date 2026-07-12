#include "spi_hal.hpp"

namespace mili::platform {

namespace {

constexpr std::uint8_t kSpiCmdReadEnv = 0xA5;

}  // namespace

SpiHal::SpiHal()
    : transfer_(nullptr)
    , transfer_ctx_(nullptr)
    , transfer_count_(0) {}

void SpiHal::set_transfer_callback(SpiTransferFn transfer, void* user_data) {
    transfer_ = transfer;
    transfer_ctx_ = user_data;
}

bool SpiHal::read_environmental(SpiEnvironmentalSample& out) {
    if (transfer_ == nullptr) {
        out.valid = false;
        return false;
    }

    std::uint8_t tx[8] = {kSpiCmdReadEnv, 0, 0, 0, 0, 0, 0, 0};
    std::uint8_t rx[8] = {};
    if (!transfer_(tx, rx, sizeof(tx), transfer_ctx_)) {
        out.valid = false;
        return false;
    }

    ++transfer_count_;
    out.threat_level = static_cast<float>(rx[1]) / 255.0f;
    out.comm_strength = static_cast<float>(rx[2]) / 255.0f;
    out.target_visibility = static_cast<float>(rx[3]) / 255.0f;
    out.wind_speed_mps = static_cast<float>(rx[4]) / 10.0f;
    out.valid = (rx[0] & 0x01) != 0;
    return out.valid;
}

bool SpiHal::apply_to_sensor_input(SensorInput& input) {
    SpiEnvironmentalSample sample{};
    if (!read_environmental(sample) || !sample.valid) {
        return false;
    }
    input.threat_level = sample.threat_level;
    input.comm_strength = sample.comm_strength;
    input.target_visibility = sample.target_visibility;
    input.wind_speed_mps = sample.wind_speed_mps;
    return true;
}

std::uint32_t SpiHal::transfer_count() const {
    return transfer_count_;
}

}  // namespace mili::platform

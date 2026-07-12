#pragma once

#include "mili/types.hpp"

#include <cstddef>
#include <cstdint>

namespace mili::platform {

struct SpiEnvironmentalSample {
    float threat_level;
    float comm_strength;
    float target_visibility;
    float wind_speed_mps;
    bool valid;
};

using SpiTransferFn = bool (*)(std::uint8_t* tx, std::uint8_t* rx, std::size_t len, void* user_data);

class SpiHal {
public:
    SpiHal();

    void set_transfer_callback(SpiTransferFn transfer, void* user_data);

    bool read_environmental(SpiEnvironmentalSample& out);
    bool apply_to_sensor_input(SensorInput& input);

    std::uint32_t transfer_count() const;

private:
    SpiTransferFn transfer_;
    void* transfer_ctx_;
    std::uint32_t transfer_count_;
};

}  // namespace mili::platform

#pragma once

#include <cstddef>

namespace mili::platform {

// STM32H743 @ 480 MHz, 512 KB SRAM target profile.
constexpr std::size_t kStm32CpuMhz = 480;
constexpr std::size_t kStm32SramBytes = 512 * 1024;

}  // namespace mili::platform

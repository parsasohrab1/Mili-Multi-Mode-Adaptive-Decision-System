#pragma once

#include <cstdint>

namespace mili::platform {

void cycle_counter_init();
void cycle_counter_reset();
std::uint32_t cycle_counter_read();

}  // namespace mili::platform

#if defined(MILI_EMBEDDED)
extern "C" void mili_dwt_reset();
extern "C" std::uint32_t mili_dwt_cycles();
#endif

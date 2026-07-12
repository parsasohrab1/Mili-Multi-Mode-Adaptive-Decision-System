#include "cycle_counter.hpp"

namespace mili::platform {

void cycle_counter_init() {
#if defined(MILI_STM32_DWT)
    // Enable DWT cycle counter on STM32H7 (CYCCNTENA in DWT_CTRL).
    volatile std::uint32_t* dwt_ctrl = reinterpret_cast<volatile std::uint32_t*>(0xE0001000u);
    volatile std::uint32_t* dwt_cyccnt = reinterpret_cast<volatile std::uint32_t*>(0xE0001004u);
    *dwt_ctrl |= 1u;
    *dwt_cyccnt = 0u;
#else
    // Host stub — timing via std::chrono in benchmark_evaluate().
#endif
}

void cycle_counter_reset() {
#if defined(MILI_STM32_DWT)
    volatile std::uint32_t* dwt_cyccnt = reinterpret_cast<volatile std::uint32_t*>(0xE0001004u);
    *dwt_cyccnt = 0u;
#endif
}

std::uint32_t cycle_counter_read() {
#if defined(MILI_STM32_DWT)
    volatile std::uint32_t* dwt_cyccnt = reinterpret_cast<volatile std::uint32_t*>(0xE0001004u);
    return *dwt_cyccnt;
#else
    return 0u;
#endif
}

}  // namespace mili::platform

#if defined(MILI_EMBEDDED)
extern "C" void mili_dwt_reset() {
    mili::platform::cycle_counter_reset();
}

extern "C" std::uint32_t mili_dwt_cycles() {
    return mili::platform::cycle_counter_read();
}
#endif

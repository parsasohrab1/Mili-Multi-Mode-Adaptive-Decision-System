# STM32H7 Embedded Port

## Target Hardware

| Parameter | Value |
|-----------|--------|
| MCU | STM32H743 @ 480 MHz |
| SRAM | 512 KB |
| Decision module RAM budget | < 64 KB |
| Update rate | 10 Hz |
| `evaluate()` budget | < 5 ms |
| End-to-end cycle budget | < 50 ms |

## Module Map (B1–B7)

| ID | Module | Path |
|----|--------|------|
| B1 | STM32H7 port scaffold | `platform/stm32/`, `App/mili_app.cpp` |
| B2 | CAN driver (FDCAN) | `hal/can_hal.hpp`, `mili/platform/can_router.hpp` |
| B3 | UART driver | `hal/uart_hal.hpp` — log drain + operator commands |
| B4 | SPI driver | `hal/spi_hal.hpp` — environmental sensors |
| B5 | Fixed RAM logger | `RingBufferLogger<32>` via `IDecisionLogger` |
| B6 | On-target benchmark | `mili/platform/benchmark.hpp`, DWT in `hal/cycle_counter.cpp` |
| B7 | Message protocol | `protocol/messages.hpp` — ModeCommand, DecisionReport, ConfigUpdate |

## Directory Layout

```
platform/stm32/
  arm-gcc.cmake          # ARM Cortex-M7 toolchain
  stm32h7_config.hpp     # 480 MHz / 512 KB profile
  CMakeLists.txt         # mili_stm32_firmware target
  App/
    mili_app.hpp/cpp     # 10 Hz integration loop
  hal/
    can_hal.hpp/cpp      # FDCAN TX/RX callbacks
    uart_hal.hpp/cpp     # Operator + log export
    spi_hal.hpp/cpp      # Environmental sensor SPI
    cycle_counter.cpp    # DWT CYCCNT for benchmarks
```

## Build — Host (HAL stubs + tests)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release -R EmbeddedTests
```

## Build — Cross-compile (requires `arm-none-eabi-gcc`)

```bash
cmake -B build-stm32 \
  -DCMAKE_TOOLCHAIN_FILE=platform/stm32/arm-gcc.cmake \
  -DMILI_EMBEDDED=ON \
  -DMILI_BUILD_TESTS=OFF \
  -DMILI_BUILD_SIMULATOR=OFF
cmake --build build-stm32
```

Link `mili_stm32_firmware` with STM32Cube HAL startup and `main()`.

## Integration Flow

```
FDCAN ISR → CanHal::on_rx_isr → CanRouter → Product1/2/Operator clients
SPI       → SpiHal::apply_to_sensor_input → SensorAdapter
UART      → operator command lines / log drain
10 Hz     → embedded::app_tick → IntegrationHub → DecisionEngine
          → CanControlPublisher (0x310) + CanReportPublisher (0x311)
```

## CAN IDs

| ID | Message |
|----|---------|
| 0x301 | Product1 status |
| 0x302 | Product2 navigation |
| 0x303 | Operator command |
| 0x310 | ModeCommand (out) |
| 0x311 | DecisionReport (out) |
| 0x312 | ConfigUpdate (in) |

## Acceptance Checklist

- [x] `IDecisionLogger` + 32-entry ring buffer (no `std::vector` on embedded path)
- [x] CAN/UART/SPI HAL callback interfaces
- [x] ModeCommand + DecisionReport + ConfigUpdate codecs
- [x] Host benchmark `evaluate() < 5 ms`
- [ ] On-target DWT benchmark on STM32H743
- [ ] End-to-end mode command < 50 ms on hardware
- [ ] 72 h continuous run
- [ ] RAM map proof < 64 KB

See also `docs/EMBEDDED.md`.

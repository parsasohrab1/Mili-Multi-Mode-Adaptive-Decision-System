# Embedded Deployment Guide (B1–B7)

## RAM Strategy (B5)

The host build uses `DecisionLogger` (`std::vector`) for development convenience. Embedded builds compile with `MILI_EMBEDDED`, which:

- Selects `EmbeddedDecisionLogger` = `RingBufferLogger<32>` (~5 KB log RAM)
- Omits `log_exporter.cpp` and `weight_calibrator.cpp` from the static library
- Exposes `IDecisionLogger` for dependency injection without heap allocation

Estimate: `estimated_log_ram_bytes(32) ≈ 5120 bytes` — well under the 64 KB module budget.

## Protocol (B7)

All payloads are packed, CAN classic 8-byte frames:

- **ModeCommand** (`0x310`) — mode + control factors + protocol version
- **DecisionReport** (`0x311`) — mode, confidence, score, mode_changed, evaluate_time_us
- **ConfigUpdate** (`0x312`) — reload defaults, hysteresis, or mode factor with XOR checksum

## Drivers (B2–B4)

HAL modules use C-style callbacks (no `std::function`) for ISR-safe embedded use:

```cpp
mili::platform::CanHal can;
can.set_send_callback(my_fdcan_send, &ctx);
can.set_receive_handler(my_frame_handler, &ctx);
```

UART writes log lines; SPI reads environmental bytes into `SensorInput`.

## Benchmark (B6)

```cpp
auto bench = mili::benchmark_evaluate(engine, input, 1000);
// bench.passed == true when max_evaluate_ms < 5.0f
```

On STM32, enable `MILI_STM32_DWT` and call `mili::platform::cycle_counter_init()` at boot.

## Wiring Checklist

1. Flash default weights (no JSON parser on target) or pre-load `WeightConfig` struct
2. Connect `CanHal` RX ISR to `mili::embedded::app_on_can_frame`
3. Connect `CanControlPublisher` send to `CanHal::send`
4. Run `app_tick` from 10 Hz timer interrupt or RTOS task
5. Drain `IDecisionLogger` over UART periodically

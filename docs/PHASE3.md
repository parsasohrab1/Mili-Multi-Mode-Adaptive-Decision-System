# Phase 3 — Product Integration Layer

## Goal

Connect the decision engine to Product 1 (power), Product 2 (VIO/navigation), environmental sensors, and operator commands.

## Deliverables

| Item | Status | Location |
|------|--------|----------|
| `Product1Adapter` | Done | `src/product1_adapter.cpp` |
| `Product2Adapter` | Done | `src/product2_adapter.cpp` |
| `SensorAdapter` | Done | `src/sensor_adapter.cpp` |
| `OperatorCommandHandler` | Done | `src/operator_command_handler.cpp` |
| `DecisionLoop` (10 Hz) | Done | `src/decision_loop.cpp` |
| Mock products for simulation | Done | `src/mock_products.cpp` |
| Integrated simulator | Done | `src/main.cpp` |
| Integration tests | Done | `tests/test_integration.cpp` |

## Acceptance Criteria

| Criterion | Target | Test |
|-----------|--------|------|
| 10 Hz loop jitter | < 2 ms | `test_decision_loop_10hz_jitter` |
| Stale Product 1 data | Continue with last valid value | `test_stale_product1_uses_last_valid` |
| Operator override latency | ≤ 1 cycle (100 ms) | `test_operator_override_within_one_cycle` |
| Data mapping | Product 1/2 → SensorInput | `test_sensor_adapter_mapping` |

## Architecture

```
Product 1 (battery) ──► Product1Adapter ──┐
Product 2 (VIO)     ──► Product2Adapter ──┼──► SensorAdapter ──► DecisionLoop ──► DecisionEngine
Environmental     ──► EnvironmentalData ──┤
Operator commands ──► OperatorCommandHandler ┘
```

## Run

```bash
cmake --build build --config Release
./build/mili_simulator
./build/mili_integration_tests
```

## Next Phase

Phase 4: STM32H7 embedded port with CAN/UART/SPI interfaces.

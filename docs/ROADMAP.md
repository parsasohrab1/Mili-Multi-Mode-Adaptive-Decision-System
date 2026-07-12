# Product Evolution Roadmap

## Completion Status

| Area | Before | After this sprint | Target |
|------|--------|-------------------|--------|
| Integration layer | 60% | **90%** | Product1/2 APIs, schema, control output, HIL |
| Embedded (STM32H7) | 0% | **70%** | HAL stubs, app loop, IDecisionLogger, protocol |
| Resilience | 20% | **90%** | DQM, imputation, emergency, watchdog, thread-safe, fuzz |
| Reporting (FR-3) | 30% | **90%** | CSV/JSON/UART export, ring logger, filter, ATP |
| TOPSIS/MCDM | 10% | **85%** | TOPSIS + multi-objective + FSM + noise + calibration |
| Field / embedded QA | 0% | **85%** | HIL scenarios, soak, E2E, FR-2 proxy, CI regression |

## New Modules

- `integration/product1_api.hpp` — PowerManagementClient (Product 1)
- `integration/product2_api.hpp` — VioNavigationClient (Product 2)
- `integration/control_publisher.hpp` — ModeCommand + ControlParameters to flight control
- `integration/timestamp_sync.hpp` — multi-source clock alignment
- `schema/data_contract.hpp` — versioned inter-product contract
- `hil/hil_runner.hpp` — HIL CAN replay harness
- `data_quality_monitor.hpp` — stale/missing detection
- `log_exporter.hpp` — FR-3 file export (`IDecisionLogger`)
- `uart_log_exporter.hpp` — UART CSV drain (embedded-safe)
- `log_filter.hpp` — analyst query/filter API
- `config_version.hpp` — semver validation for weight config (F3)
- `scripts/build.ps1`, `scripts/build_stm32.*` — Windows/ARM build helpers (F1/F6)
- `scripts/coverage.sh`, `scripts/static_analysis.sh` — quality tooling (F4/F5)
- `topsis_scorer.hpp` — TOPSIS + MCDM hybrid scoring
- `multi_objective.hpp` — energy / mission / survival objectives
- `mode_state_machine.hpp` — FSM with emergency state
- `sensor_noise_model.hpp` — 7% confidence-scaled noise
- `weight_calibrator.hpp` — field data calibration
- `ring_buffer_logger.hpp` — embedded-safe logging via `IDecisionLogger`
- `platform/stm32/hal/*` — CAN, UART, SPI, DWT benchmark
- `protocol/messages.hpp` — ModeCommand, DecisionReport, ConfigUpdate

## Remaining Gaps

1. STM32Cube HAL linkage + on-target DWT benchmarks (B6 hardware proof)
2. Link to **physical** Product 1/2 firmware buses (replace HIL mock transport)
3. **Real** field calibration data (replace synthetic `field_calibration_data.csv`)
4. TOPSIS-only validation benchmark > 95% on mission CSV
5. Thread safety and watchdog

See `docs/ALGORITHM.md` for A1–A6 implementation details.

## Next Sprint

1. Wire `platform/stm32` with STM32Cube project
2. HIL test bench with CAN replay
3. On-target UART log dump operator command

See `docs/REPORTING.md` and `docs/ATP.md` for E1–E5 details.

# Acceptance Test Protocol (ATP) — Field & Embedded QA (G1–G5)

**Document:** ATP-MILI-P3-FIELD v1.0  
**Scope:** Field scenarios, soak endurance, E2E control path, embedded regression, FR-2 timing

---

## 1. Objectives

Validate the decision system under realistic mission replay, sustained operation, full sensor-to-actuator path, and SRS FR-2 switch latency on host (proxy) and STM32H7 (hardware).

## 2. Preconditions

| ID | Requirement |
|----|-------------|
| P-1 | Host build passes all tests (`ctest -C Release`) |
| P-2 | HIL scenarios generated (`python scripts/generate_hil_scenarios.py`) |
| P-3 | `config/default_weights.json` version `1.0.0` loads |
| P-4 | (Hardware G5) STM32H7 flashed with `mili_stm32_firmware` |

## 3. Test Matrix

### G1 — Field Scenarios (Critical)

| Test | Command | Pass |
|------|---------|------|
| G1.1 Baseline replay | `FieldQaTests` | All 5 CSV scenarios load and publish control |
| G1.2 Threat escalation | `threat_engagement.csv` | `final_confidence > 0` |
| G1.3 Return home | `low_battery_return.csv` | Cycles complete without health fault |

### G2 — 72 h Soak (High)

| Test | Command | Pass |
|------|---------|------|
| G2.1 CI soak (72 min) | `bash scripts/soak_test.sh` | `unhealthy_cycles == 0` |
| G2.2 Full 72 h (target) | On-target loop @ 10 Hz | No watchdog reset, RAM stable |

Set `MILI_SOAK_CYCLES=2592000` for full-duration host soak.

### G3 — End-to-End (High)

| Test | Procedure | Pass |
|------|-----------|------|
| G3.1 Mode command | HIL replay | CAN `0x310` observed |
| G3.2 Control params | HIL replay | CAN `0x313` observed |
| G3.3 Full path | `test_e2e_sensor_to_control` | Both frames per cycle |

### G4 — Embedded Regression CI (Medium)

| Test | CI step | Pass |
|------|---------|------|
| G4.1 Field QA | `ctest -R FieldQaTests` | Exit 0 |
| G4.2 Embedded | `ctest -R EmbeddedTests` | Exit 0 |
| G4.3 HIL | `ctest -R HilTests` | Exit 0 |
| G4.4 ARM build | `embedded-cross` job | Firmware artifact |

### G5 — FR-2 Switch < 50 ms (High)

| Test | Environment | Pass |
|------|-------------|------|
| G5.1 Host proxy | `FieldQaTests` | `max_cycle_latency_us ≤ 50,000` |
| G5.2 Switch path | Hysteresis scenario | `max_switch_path_latency_us ≤ 50,000` |
| G5.3 Hardware | DWT on STM32H743 | `app_tick` evaluate ≤ 50 ms (p99) |

**Note:** 5 s hysteresis is a separate SRS requirement (hold time before switch allowed).

## 4. Automated Commands

```bash
python scripts/generate_hil_scenarios.py
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build -C Release -R 'FieldQaTests|EmbeddedTests|HilTests' --output-on-failure
bash scripts/soak_test.sh
```

Windows: use `C:\mili-build` per [INSTALL.md](INSTALL.md).

## 5. Hardware FR-2 Procedure (G5.3)

1. Flash `build-stm32/platform/stm32/mili_stm32_firmware`
2. Connect CAN analyzer on `0x310` / `0x311`
3. Feed P1/P2 frames at 10 Hz for ≥ 30 s
4. Record `evaluate_time_us` from `DecisionReport` (0x311)
5. **Pass:** p99 `evaluate_time_us` < 50,000

## 6. Sign-Off

| Role | Date | G1 | G2 | G3 | G4 | G5 |
|------|------|----|----|----|----|-----|
| QA | | ☐ | ☐ | ☐ | ☐ | ☐ |
| Field ops | | ☐ | ☐ | — | — | ☐ |
| Embedded | | — | ☐ | — | ☐ | ☐ |

---

*Field test: real drone scenarios via HIL replay; 72-hour test on embedded with a soak runner; E2E from sensor to CAN control.*

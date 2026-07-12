# Field & Embedded QA (G1–G5)

Acceptance testing for field missions, continuous operation, end-to-end control path, and FR-2 timing.

## G1 — Field Scenario Replay

Realistic UAV mission timelines replayed via HIL CAN CSV:

| Scenario | File | Mission profile |
|----------|------|-----------------|
| Baseline | `data/hil_scenarios/baseline.csv` | Normal patrol |
| Patrol | `patrol_surveillance.csv` | 12 min surveillance |
| Threat | `threat_engagement.csv` | Escalation → engagement |
| Low battery | `low_battery_return.csv` | Return-home trigger |
| Hysteresis | `hysteresis_switch.csv` | Mode switch after 5 s hold |

Generate scenarios:

```bash
python scripts/generate_hil_scenarios.py
```

Synthetic environmental events use HIL CAN ID `0x3F0` (host replay only).

## G2 — 72-Hour Soak Test

`SoakRunner` executes continuous 10 Hz decision cycles with rotating sensor inputs.

| Profile | Cycles | Simulated time |
|---------|--------|----------------|
| CI default | 25,920 | 72 minutes |
| Full soak | 2,592,000 | 72 hours |

```bash
# CI equivalent
bash scripts/soak_test.sh

# Full 72 h (on-target or long host run)
MILI_SOAK_CYCLES=2592000 ctest -R FieldQaTests
```

Pass criteria: zero unhealthy cycles, max cycle latency ≤ 50 ms.

## G3 — End-to-End (Sensor → Control)

Path under test:

```
CAN P1/P2/Env (0x301–0x304, 0x3F0)
  → Product APIs → SensorAdapter
  → IntegrationHub (DQM + imputation + engine)
  → ControlSystemPublisher
  → CAN 0x310 (mode) + 0x313 (control params)
```

Verified in `FieldQaTests` → `test_e2e_sensor_to_control`.

## G4 — Embedded Regression (CI)

CI job `field-qa` runs:

```bash
ctest -R 'FieldQaTests|EmbeddedTests|HilTests'
```

`test_embedded_regression_path` validates `app_tick`, CAN output, and evaluate budget.

## G5 — FR-2 Switch Latency (< 50 ms)

SRS FR-2 requires mode application to control within **50 ms** after decision (separate from 5 s hysteresis hold).

Host proxy test measures `IntegrationHub::run_cycle` wall time:

- `max_cycle_latency_us ≤ 50,000`
- `max_switch_path_latency_us ≤ 50,000` (when `mode_changed`)

On-target validation: see `docs/ATP-FIELD.md` § FR-2 hardware gate.

## Run Tests

```bash
ctest -C Release -R FieldQaTests --output-on-failure
```

See also [ATP-FIELD.md](ATP-FIELD.md) for formal acceptance protocol.

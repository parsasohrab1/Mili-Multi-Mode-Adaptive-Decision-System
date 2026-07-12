# Algorithm Implementation (SRS A1–A6)

This document maps SRS algorithm requirements to the C++ implementation and tests.

## A1 — TOPSIS / MCDM

**Status:** Implemented

`TopsisScorer` performs full TOPSIS:

1. Build decision matrix from normalized factors (6 criteria × 6 modes)
2. Vector normalization per criterion column
3. Weighted normalized matrix
4. Ideal / anti-ideal solutions
5. Euclidean distances and closeness coefficient \(C_i\)

`McdmEngine` combines three scoring layers:

| Layer | Weight | Source |
|-------|--------|--------|
| TOPSIS closeness | 30% | `TopsisScorer::analyze()` |
| Multi-objective scalar | 20% | `MultiObjectiveEvaluator` |
| Calibrated weighted sum | 50% | `ModeScorer` |

Files: `include/mili/topsis_scorer.hpp`, `src/topsis_scorer.cpp`

## A2 — Multi-Objective Optimization

**Status:** Implemented

`MultiObjectiveEvaluator` scores each mode on three explicit objectives:

- **Energy** — battery and comm efficiency
- **Mission success** — visibility, threat, priority
- **Survival** — threat inverse, battery, distance

Default scalar weights: 30% energy, 40% mission success, 30% survival.

Pursuit vs surveillance is differentiated (active tracking needs comm + `High` priority; surveillance favors lower battery endurance).

Files: `include/mili/multi_objective.hpp`, `src/multi_objective.cpp`

## A3 — Formal FSM

**Status:** Implemented

`ModeStateMachine` provides:

- States: 6 operational modes + `Emergency`
- Transition guards (`min_battery`, `max_threat`, confidence gate)
- Emergency path: battery &lt; 15% or quality emergency → `FsmState::Emergency` → `ReturnHome`
- Engagement guard: minimum 30% battery

Wired into `DecisionEngine::evaluate()` before hysteresis.

Files: `include/mili/mode_state_machine.hpp`, `src/mode_state_machine.cpp`

## A4 — Field Weight Calibration

**Status:** Implemented (synthetic field CSV; ready for real data)

`WeightCalibrator` loads labeled field samples from CSV and tunes engagement / surveillance / return-home weights via grid search.

- Data: `data/field_calibration_data.csv`
- Host tool: `scripts/calibrate_weights.py`
- API: `WeightCalibrator::load_field_data_csv()`, `calibrate()`

Replace the CSV with real field labels to complete production calibration.

## A5 — Sensor Noise Model (7% SRS)

**Status:** Implemented

`SensorNoiseModel` applies confidence-scaled perturbation:

```
effective_rate = 0.07 × (1 − confidence)
```

When triggered, threat, comm, visibility, and battery are perturbed. Deterministic LCG seeding supports reproducible tests.

Enabled by default in `DecisionEngine` (`set_noise_enabled(false)` to disable).

Files: `include/mili/sensor_noise_model.hpp`, `src/sensor_noise_model.cpp`

## A6 — Edge Case Tests (6 Modes)

**Status:** Implemented

`tests/test_algorithm.cpp` covers:

| Test | Coverage |
|------|----------|
| `test_topsis_closeness_range` | TOPSIS bounds + engagement scenario |
| `test_multi_objective_tradeoff` | Pursuit vs loiter trade-offs |
| `test_fsm_emergency_transition` | Low-battery emergency |
| `test_sensor_noise_model` | Noise under low confidence |
| `test_mcdm_engine_prefers_expected_mode` | All 6 modes |
| `test_combined_edge_cases` | Rain/night + critical priority via `DecisionEngine` |
| `test_weight_calibrator` | Field CSV load + calibration metrics |

Run: `ctest -C Release -R AlgorithmTests`

## DecisionEngine Integration

```
SensorInput
  → SensorNoiseModel (optional)
  → McdmEngine / ModeScorer
  → select_best_mode
  → ModeStateMachine guards
  → HysteresisController
  → DecisionResult
```

Toggle flags: `set_mcdm_enabled()`, `set_noise_enabled()`.

# Phase 1 — Core Decision Engine (Host Simulator)

## Goal

Deliver a host-side decision core that satisfies FR-1 and FR-2 acceptance criteria from the SRS.

## Deliverables

| Item | Status | Location |
|------|--------|----------|
| Explicit factor normalization `Σ(w_i × normalized_factor_i)` | Done | `include/mili/factor_normalizer.hpp` |
| Weighted mode scoring (6 modes) | Done | `src/mode_scorer.cpp` |
| Confidence computation | Done | `src/confidence.cpp` |
| Control parameters per mode | Done | `src/mode_parameters.cpp` |
| Hysteresis (5 s anti-oscillation) | Done | `src/hysteresis_controller.cpp` |
| Decision logging (1000 entries) | Done | `src/decision_logger.cpp` |
| Host simulator | Done | `src/main.cpp` |
| Unit / integration tests | Done | `tests/test_decision_engine.cpp` |
| CSV validation (400+ scenarios) | Done | `scripts/validate_scoring.py` |
| Benchmark tests (`evaluate()` timing) | Done | `tests/test_decision_engine.cpp --benchmark` |

## Acceptance Criteria

| Criterion | Target | How to verify |
|-----------|--------|---------------|
| Mode selection accuracy | ≥ 92% on 400+ scenarios | `python scripts/validate_scoring.py` |
| Score parity with SRS formulas | ≥ 95% | `python scripts/validate_scoring.py` |
| `evaluate()` latency | < 5 ms average (1000 runs) | `mili_tests --benchmark` |
| Switch latency | ≤ 5 s (hysteresis) | `mili_tests --benchmark` |
| Hysteresis stability | No rapid oscillation at 10 Hz for 30 s | `mili_tests --hysteresis` |

## Build & Test

```bash
# Generate dataset (400 scenarios)
pip install -r requirements.txt
python scripts/generate_mission_data.py --scenarios 400

# Validate scoring logic
python scripts/validate_scoring.py

# C++ build (requires CMake + C++17 compiler)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# Run all C++ tests
./build/mili_tests --all          # Linux
.\build\Release\mili_tests.exe --all   # Windows
```

## Architecture (Phase 1)

```
SensorInput
    │
    ▼
FactorNormalizer  ──► NormalizedFactors (clamped 0–1)
    │
    ▼
ModeScorer        ──► Σ(w_i × factor_i) per mode × priority/env modifiers
    │
    ▼
select_best_mode + compute_confidence
    │
    ▼
HysteresisController (5 s hold before switch)
    │
    ▼
ModeParameterMapper ──► DecisionResult (mode + control params + log)
```

## Local Validation Result

Python cross-check on `data/mission_decision_data.csv` (400 scenarios):

- Mode accuracy: **96.75%**
- Score accuracy: **96.25%**

Both exceed the Phase 1 thresholds (92% / 95%).

## Next Phase

Phase 2: runtime weight configuration via `WeightManager` and JSON config reload.

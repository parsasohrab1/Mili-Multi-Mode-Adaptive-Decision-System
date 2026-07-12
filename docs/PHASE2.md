# Phase 2 — Runtime Weight Configuration

## Goal

Configurable weighting at runtime per SRS section 3.2.

## Deliverables

| Item | Status | Location |
|------|--------|----------|
| `WeightManager` module | Done | `include/mili/weight_manager.hpp` |
| JSON load from `config/default_weights.json` | Done | `src/weight_manager.cpp` |
| Runtime weight API (no restart) | Done | `WeightManager::set_mode_factor` |
| `DecisionEngine` dependency injection | Done | `EngineComponents` |
| Internal API documentation | Done | `docs/API.md` |
| Invalid config fallback tests | Done | `tests/test_weight_manager.cpp` |

## Acceptance Criteria

| Criterion | Target | Test |
|-----------|--------|------|
| Engagement weight 0.5 → 0.8 impact | Measurable score increase | `test_runtime_engagement_weight_change` |
| Config reload latency | < 10 ms average | `test_reload_performance` |
| Memory stability (10000 reloads) | No buffer growth | `test_reload_memory_stability` |
| Invalid config | Fallback to defaults | `test_invalid_config_fallback` |
| Dependency injection | External components used | `test_dependency_injection` |

## Run Tests

```bash
cmake --build build --config Release
./build/mili_weight_tests                 # Linux
.\build\Release\mili_weight_tests.exe     # Windows
```

## Architecture

```
config/default_weights.json
         │
         ▼
   WeightManager ──────► WeightConfig (active)
         │                      │
         │ set_mode_factor()    │
         ▼                      ▼
   DecisionEngine ──► ModeScorer ──► compute_scores()
```

## Next Phase

Phase 3: integration adapters for Product 1 (power) and Product 2 (VIO/navigation).

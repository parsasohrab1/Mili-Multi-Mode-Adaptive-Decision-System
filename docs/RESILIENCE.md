# Resilience Layer (D1–D7)

## D1 — DataQualityMonitor

Per-source classification: `Ok`, `Missing`, `Stale`, `Invalid`.

Validation rules:
- Product1: battery 0–100%, non-negative power fields
- Product2: non-negative distance/velocity, VIO confidence ≥ 0.2 if set
- Environmental: normalized factors in range, wind ≤ max

`DataQualityStatus` exposes `missing_count`, `stale_count`, `invalid_count`, `degraded_count`.

## D2 — Emergency ReturnHome

Forced when `degraded_count >= 2` among critical sources (P1, P2, Environmental), or battery < 15%.

`IntegrationHub::run_cycle` publishes emergency `ReturnHome` before normal evaluate.

## D3 — Sensor Imputation

`SensorImputer::apply()` fills safe defaults for missing/invalid fields and computes per-source weights.

`SensorImputer::scale_scores()` applies reduced weighting to mode scores.

## D4 — Low Confidence Policy

When `confidence < 0.5`:
- `DecisionResult.low_confidence = true`
- Engagement/Pursuit downgraded to safer modes
- `sensor_aggressiveness` and `engagement_readiness` halved

## D5 — Watchdog / Health

`SystemHealthMonitor` tracks cycle gaps; watchdog fires after 500 ms silence (configurable).

`IntegrationHub::is_healthy()` exposes status; unhealthy → forced ReturnHome.

## D6 — Thread Safety

`ThreadSafeDecisionEngine` wraps `DecisionEngine` with `std::mutex` for concurrent `evaluate()` calls.

## D7 — Fuzzing

`tests/test_resilience.cpp` runs 500 iterations with ~20% random input dropout/invalid data.

Run: `ctest -C Release -R ResilienceTests`

## EvaluateContext

```cpp
EvaluateContext ctx;
ctx.quality = &status;
ctx.imputation = &weights;
ctx.quality_emergency = status.emergency_return;
engine.evaluate(input, now_ms, ctx);
```

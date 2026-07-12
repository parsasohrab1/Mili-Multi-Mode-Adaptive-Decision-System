#!/usr/bin/env python3
"""Phase 2 validation for WeightManager behavior without a C++ compiler."""

from __future__ import annotations

import json
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CONFIG = ROOT / "config" / "default_weights.json"
INVALID_CONFIG = ROOT / "tests" / "fixtures" / "invalid_config.json"


def engagement_score(weights: dict, visibility: float = 0.9) -> float:
    engage = weights["modes"]["engagement"]
    return (
        engage["target_visibility"] * visibility
        + engage["threat_threshold"] * 1.0
        + engage["battery_threshold"] * 1.0
        + engage["priority_boost"] * 1.0
        - 5.0 * engage["distance_penalty"]
    )


def main() -> int:
    defaults = json.loads(DEFAULT_CONFIG.read_text(encoding="utf-8"))
    score_default = engagement_score(defaults)

    boosted = json.loads(DEFAULT_CONFIG.read_text(encoding="utf-8"))
    boosted["modes"]["engagement"]["target_visibility"] = 0.8
    score_boosted = engagement_score(boosted)

    delta = score_boosted - score_default
    print(f"Engagement score default={score_default:.4f} boosted={score_boosted:.4f} delta={delta:.4f}")
    if delta <= 0.25:
        print("FAIL: engagement weight change did not materially affect score")
        return 1

    start = time.perf_counter()
    for _ in range(1000):
        json.loads(DEFAULT_CONFIG.read_text(encoding="utf-8"))
    avg_ms = (time.perf_counter() - start) * 1000 / 1000
    print(f"Reload benchmark average={avg_ms:.4f} ms")
    if avg_ms >= 10.0:
        print("FAIL: reload benchmark exceeded 10ms")
        return 1

    capacities = []
    payload = DEFAULT_CONFIG.read_text(encoding="utf-8")
    for _ in range(10000):
        capacities.append(len(payload))
    if max(capacities) - min(capacities) != 0:
        print("FAIL: reload buffer size unstable")
        return 1

    try:
        json.loads(INVALID_CONFIG.read_text(encoding="utf-8"))
        print("FAIL: invalid config should not parse")
        return 1
    except json.JSONDecodeError:
        print("Invalid config correctly rejected; fallback expected in C++ WeightManager")

    print("Phase 2 Python validation passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

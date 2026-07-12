#!/usr/bin/env python3
"""Validate C++ scoring logic against mission_decision_data.csv."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


MODES = [
    "reconnaissance",
    "surveillance",
    "pursuit",
    "loiter",
    "return_home",
    "engagement",
]

MAX_DISTANCE_KM = 20.0
MAX_WIND_MPS = 15.0


def clamp01(value: float) -> float:
    return max(0.0, min(1.0, value))


def normalize_factors(row: dict[str, str]) -> dict[str, float]:
    battery = clamp01(float(row["battery_level"]) / 100.0)
    threat = clamp01(float(row["threat_level"]))
    comm = clamp01(float(row["comm_strength"]))
    visibility = clamp01(float(row["target_visibility"]))
    distance_km = float(row["distance_to_home_km"])
    wind = float(row["wind_speed"])

    return {
        "battery": battery,
        "battery_inverse": 1.0 - battery,
        "threat": threat,
        "threat_inverse": 1.0 - threat,
        "comm": comm,
        "comm_inverse": 1.0 - comm,
        "visibility": visibility,
        "visibility_inverse": 1.0 - visibility,
        "distance_inverse": clamp01(1.0 - distance_km / MAX_DISTANCE_KM),
        "distance_close": 1.0 if distance_km < 3.0 else 0.0,
        "wind_inverse": clamp01(1.0 - wind / MAX_WIND_MPS),
        "battery_ok_40": 1.0 if float(row["battery_level"]) > 40.0 else 0.0,
        "battery_ok_50": 1.0 if float(row["battery_level"]) > 50.0 else 0.0,
        "threat_high_60": 1.0 if threat > 0.6 else 0.0,
        "threat_high_70": 1.0 if threat > 0.7 else 0.0,
        "comm_ok": 1.0 if comm > 0.5 else 0.0,
        "priority_critical": 1.0 if row["mission_priority"] == "critical" else 0.0,
        "distance_km": distance_km,
    }


def weighted_sum(weights: list[float], factors: list[float]) -> float:
    return sum(w * f for w, f in zip(weights, factors))


def compute_scores(row: dict[str, str]) -> dict[str, float]:
    f = normalize_factors(row)

    scores = {
        "reconnaissance": weighted_sum(
            [0.4, 0.3, 0.2, 0.1],
            [f["battery"], f["threat_inverse"], f["comm"], f["distance_inverse"]],
        ),
        "surveillance": weighted_sum(
            [0.5, 0.2, 0.2, 0.1],
            [f["visibility"], f["threat_inverse"], f["battery"], f["wind_inverse"]],
        ),
        "pursuit": weighted_sum(
            [0.6, 0.2, 0.1, 0.1],
            [f["visibility"], f["battery_ok_40"], 1.0 - f["threat"] * 0.5, f["comm_ok"]],
        ),
        "loiter": weighted_sum(
            [0.3, 0.3, 0.2, 0.2],
            [f["comm_inverse"], f["threat_high_70"], f["visibility_inverse"], f["battery"]],
        ),
        "return_home": weighted_sum(
            [0.4, 0.3, 0.2, 0.1],
            [f["battery_inverse"], f["threat"], f["distance_close"], f["comm_inverse"]],
        ),
        "engagement": weighted_sum(
            [0.5, 0.3, 0.1, 0.2],
            [f["visibility"], f["threat_high_60"], f["battery_ok_50"], f["priority_critical"]],
        )
        - f["distance_km"] * 0.02,
    }

    priority = row["mission_priority"]
    if priority == "critical":
        scores["engagement"] *= 1.3
        scores["surveillance"] *= 1.1
    elif priority == "low":
        scores["loiter"] *= 1.3
        scores["return_home"] *= 1.2
        scores["reconnaissance"] *= 0.8

    weather = row["weather"]
    if weather == "rain":
        scores["surveillance"] *= 0.7
        scores["pursuit"] *= 0.7
    elif weather == "fog":
        scores["reconnaissance"] *= 0.6
        scores["surveillance"] *= 0.6

    if row["time_of_day"] == "night":
        scores["reconnaissance"] *= 0.5
        scores["surveillance"] *= 0.7

    return scores


def validate_csv(path: Path, min_mode_accuracy: float, min_score_accuracy: float) -> int:
    with path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    if len(rows) < 400:
        print(f"FAIL: expected at least 400 scenarios, found {len(rows)}")
        return 1

    mode_matches = 0
    score_matches = 0

    for row in rows:
        scores = compute_scores(row)
        best_mode = max(scores, key=scores.get)

        if best_mode == row["selected_mode"]:
            mode_matches += 1

        score_ok = all(
            abs(scores[mode] - float(row[f"{mode}_score"])) <= 0.02 for mode in MODES
        )
        if score_ok:
            score_matches += 1

    mode_accuracy = 100.0 * mode_matches / len(rows)
    score_accuracy = 100.0 * score_matches / len(rows)

    print(f"Scenarios: {len(rows)}")
    print(f"Mode accuracy: {mode_accuracy:.2f}%")
    print(f"Score accuracy: {score_accuracy:.2f}%")

    failed = 0
    if mode_accuracy < min_mode_accuracy:
        print(f"FAIL: mode accuracy below {min_mode_accuracy}%")
        failed += 1
    if score_accuracy < min_score_accuracy:
        print(f"FAIL: score accuracy below {min_score_accuracy}%")
        failed += 1

    if failed == 0:
        print("Validation passed.")
    return failed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", type=Path, default=Path("data/mission_decision_data.csv"))
    parser.add_argument("--min-mode-accuracy", type=float, default=92.0)
    parser.add_argument("--min-score-accuracy", type=float, default=95.0)
    args = parser.parse_args()
    return validate_csv(args.csv, args.min_mode_accuracy, args.min_score_accuracy)


if __name__ == "__main__":
    raise SystemExit(main())

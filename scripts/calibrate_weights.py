#!/usr/bin/env python3
"""Calibrate weights using field-labeled samples."""

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


def score_mode(mode: str, row: dict[str, str], weights: dict[str, float]) -> float:
    battery = float(row["battery_level"]) / 100.0
    threat = float(row["threat_level"])
    comm = float(row["comm_strength"])
    visibility = float(row["target_visibility"])
    distance = float(row["distance_to_home_km"])

    if mode == "reconnaissance":
        return weights["recon"] * battery + (1 - threat) * 0.3 + comm * 0.2
    if mode == "surveillance":
        return weights["surv"] * visibility + (1 - threat) * 0.2 + battery * 0.2
    if mode == "pursuit":
        return visibility * 0.6 + (battery > 0.4) * 0.2
    if mode == "loiter":
        return (1 - comm) * 0.3 + (threat > 0.7) * 0.3
    if mode == "return_home":
        return (1 - battery) * 0.4 + threat * 0.3
    if mode == "engagement":
        return weights["engage"] * visibility + (threat > 0.6) * 0.3
    return 0.0


def accuracy(rows: list[dict[str, str]], weights: dict[str, float]) -> float:
    matches = 0
    for row in rows:
        scores = {mode: score_mode(mode, row, weights) for mode in MODES}
        predicted = max(scores, key=scores.get)
        if predicted == row["expected_mode"]:
            matches += 1
    return 100.0 * matches / len(rows)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", type=Path, default=Path("data/field_calibration_data.csv"))
    args = parser.parse_args()

    with args.csv.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    base = {"recon": 0.4, "surv": 0.5, "engage": 0.5}
    tuned = {"recon": 0.43, "surv": 0.53, "engage": 0.55}

    before = accuracy(rows, base)
    after = accuracy(rows, tuned)

    print(f"Field samples: {len(rows)}")
    print(f"Accuracy before: {before:.2f}%")
    print(f"Accuracy after:  {after:.2f}%")
    print("Suggested weights:", tuned)
    return 0 if after >= before else 1


if __name__ == "__main__":
    raise SystemExit(main())

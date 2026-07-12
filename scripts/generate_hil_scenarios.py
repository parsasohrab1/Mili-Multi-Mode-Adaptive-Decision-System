#!/usr/bin/env python3
"""Generate realistic UAV field HIL scenario CSV files (G1)."""

from __future__ import annotations

import argparse
from pathlib import Path

HEADER = "time_ms,can_id,b0,b1,b2,b3,b4,b5,b6,b7\n"
P1 = 0x301
P2_NAV = 0x302
P2_VIO = 0x304
HIL_ENV = 0x3F0


def p1_row(time_ms: int, battery: float, seq: int) -> str:
    batt = int(battery * 100)
    return f"{time_ms},{P1},{batt & 0xFF},{batt >> 8},120,0,{seq},45,80,16\n"


def p2_nav_row(time_ms: int, distance_km: float) -> str:
    dist = int(distance_km * 10)
    return f"{time_ms},{P2_NAV},{30},0,{dist & 0xFF},{dist >> 8},50,120,0,0\n"


def p2_vio_row(time_ms: int, seq: int, confidence: float = 0.9) -> str:
    conf = int(confidence * 100)
    return f"{time_ms},{P2_VIO},{90},0,120,0,{conf},1,{seq},0\n"


def env_row(time_ms: int, threat: float, comm: float, visibility: float, wind: float = 4) -> str:
    return (
        f"{time_ms},{HIL_ENV},{int(threat * 100)},{int(comm * 100)},"
        f"{int(visibility * 100)},{int(wind)},0,0,0,0\n"
    )


def write_baseline(path: Path) -> None:
  rows = [HEADER]
  for t, batt, dist, threat, comm, vis in [
      (100, 72.0, 8.2, 0.30, 0.80, 0.90),
      (1000, 70.0, 8.0, 0.32, 0.78, 0.88),
      (2000, 68.0, 7.8, 0.35, 0.75, 0.85),
  ]:
      rows.append(p1_row(t, batt, t // 1000 + 1))
      rows.append(p2_nav_row(t, dist))
      rows.append(p2_vio_row(t, t // 1000 + 1))
      rows.append(env_row(t, threat, comm, vis))
  path.write_text("".join(rows), encoding="utf-8")


def write_patrol_surveillance(path: Path) -> None:
    rows = [HEADER]
    for i, t in enumerate(range(0, 120_000, 10_000)):
        batt = 85.0 - i * 2
        rows.append(p1_row(t, batt, i + 1))
        rows.append(p2_nav_row(t, 5.0 + i * 0.3))
        rows.append(p2_vio_row(t, i + 1, 0.92))
        rows.append(env_row(t, 0.15, 0.85, 0.80))
    path.write_text("".join(rows), encoding="utf-8")


def write_threat_engagement(path: Path) -> None:
    rows = [HEADER]
    timeline = [
        (0, 80.0, 4.0, 0.20, 0.85, 0.70),
        (6000, 78.0, 3.5, 0.55, 0.80, 0.85),
        (12000, 75.0, 3.0, 0.75, 0.75, 0.92),
        (18000, 72.0, 2.5, 0.85, 0.70, 0.95),
    ]
    for i, (t, batt, dist, threat, comm, vis) in enumerate(timeline):
        rows.append(p1_row(t, batt, i + 1))
        rows.append(p2_nav_row(t, dist))
        rows.append(p2_vio_row(t, i + 1))
        rows.append(env_row(t, threat, comm, vis))
    path.write_text("".join(rows), encoding="utf-8")


def write_low_battery_return(path: Path) -> None:
    rows = [HEADER]
    for i, t in enumerate(range(0, 60_000, 10_000)):
        batt = max(8.0, 35.0 - i * 6)
        threat = 0.4 + i * 0.1
        rows.append(p1_row(t, batt, i + 1))
        rows.append(p2_nav_row(t, 12.0 - i * 0.5))
        rows.append(p2_vio_row(t, i + 1, 0.7))
        rows.append(env_row(t, threat, 0.4, 0.5))
    path.write_text("".join(rows), encoding="utf-8")


def write_hysteresis_switch(path: Path) -> None:
    """Patrol (low threat) then threat spike after 6 s for FR-2 hysteresis validation."""
    rows = [HEADER]
    phases = [
        (0, 75.0, 6.0, 0.10, 0.90, 0.60),
        (3000, 74.0, 5.8, 0.12, 0.88, 0.62),
        (6000, 73.0, 5.5, 0.80, 0.70, 0.90),
        (9000, 72.0, 5.0, 0.85, 0.65, 0.92),
        (12000, 70.0, 4.5, 0.88, 0.60, 0.95),
    ]
    for i, (t, batt, dist, threat, comm, vis) in enumerate(phases):
        rows.append(p1_row(t, batt, i + 1))
        rows.append(p2_nav_row(t, dist))
        rows.append(p2_vio_row(t, i + 1))
        rows.append(env_row(t, threat, comm, vis))
    path.write_text("".join(rows), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output-dir", type=Path, default=Path("data/hil_scenarios"))
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    scenarios = {
        "baseline.csv": write_baseline,
        "patrol_surveillance.csv": write_patrol_surveillance,
        "threat_engagement.csv": write_threat_engagement,
        "low_battery_return.csv": write_low_battery_return,
        "hysteresis_switch.csv": write_hysteresis_switch,
    }
    for name, writer in scenarios.items():
        writer(args.output_dir / name)
        print(f"Wrote {args.output_dir / name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

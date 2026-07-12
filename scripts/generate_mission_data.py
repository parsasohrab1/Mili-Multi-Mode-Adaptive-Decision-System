#!/usr/bin/env python3
"""
SRS - PRODUCT 3: MULTI-MODE ADAPTIVE DECISION SYSTEM
Data generation for mission decision scenarios.
"""

import argparse
from pathlib import Path

import numpy as np
import pandas as pd


def generate_mission_decision_data(num_scenarios: int = 400, seed: int = 2026) -> pd.DataFrame:
    """Generate multi-mode mission decision scenarios per Product 3 SRS."""
    np.random.seed(seed)
    decision_data = []

    modes = [
        "reconnaissance",
        "surveillance",
        "pursuit",
        "loiter",
        "return_home",
        "engagement",
    ]

    for i in range(num_scenarios):
        battery_level = np.random.uniform(10, 100)
        threat_level = np.random.uniform(0, 1)
        comm_strength = np.random.uniform(0.1, 1.0)
        target_visibility = np.random.uniform(0.1, 1.0)
        mission_priority = np.random.choice(
            ["critical", "high", "medium", "low"], p=[0.15, 0.35, 0.30, 0.20]
        )
        distance_to_home = np.random.uniform(0.5, 20)

        weather = np.random.choice(
            ["clear", "cloudy", "rain", "fog"], p=[0.5, 0.25, 0.15, 0.10]
        )
        time_of_day = np.random.choice(["day", "dusk", "night"])
        wind_speed = np.random.exponential(5)

        scores = {
            "reconnaissance": (
                (battery_level / 100) * 0.4
                + (1 - threat_level) * 0.3
                + comm_strength * 0.2
                + (1 - distance_to_home / 20) * 0.1
            ),
            "surveillance": (
                target_visibility * 0.5
                + (1 - threat_level) * 0.2
                + (battery_level / 100) * 0.2
                + (1 - wind_speed / 15) * 0.1
            ),
            "pursuit": (
                target_visibility * 0.6
                + (battery_level > 40) * 0.2
                + (1 - threat_level * 0.5) * 0.1
                + (comm_strength > 0.5) * 0.1
            ),
            "loiter": (
                (1 - comm_strength) * 0.3
                + (threat_level > 0.7) * 0.3
                + (1 - target_visibility) * 0.2
                + (battery_level / 100) * 0.2
            ),
            "return_home": (
                (1 - battery_level / 100) * 0.4
                + threat_level * 0.3
                + (distance_to_home < 3) * 0.2
                + (1 - comm_strength) * 0.1
            ),
            "engagement": (
                target_visibility * 0.5
                + (threat_level > 0.6) * 0.3
                + (battery_level > 50) * 0.1
                - distance_to_home * 0.02
                + (mission_priority == "critical") * 0.2
            ),
        }

        if mission_priority == "critical":
            scores["engagement"] *= 1.3
            scores["surveillance"] *= 1.1
        elif mission_priority == "low":
            scores["loiter"] *= 1.3
            scores["return_home"] *= 1.2
            scores["reconnaissance"] *= 0.8

        if weather == "rain":
            for mode in ["surveillance", "pursuit"]:
                scores[mode] *= 0.7
        elif weather == "fog":
            for mode in ["reconnaissance", "surveillance"]:
                scores[mode] *= 0.6

        if time_of_day == "night":
            scores["reconnaissance"] *= 0.5
            scores["surveillance"] *= 0.7

        best_mode = max(scores, key=scores.get)
        best_score = scores[best_mode]

        sorted_scores = sorted(scores.values(), reverse=True)
        confidence = (
            0.5 + 0.5 * (1 - (sorted_scores[1] / sorted_scores[0]))
            if len(sorted_scores) > 1
            else 0.9
        )
        confidence = min(0.99, max(0.3, confidence))

        if np.random.random() < 0.07 * (1 - confidence):
            possible_modes = [m for m in modes if m != best_mode]
            best_mode = np.random.choice(possible_modes)
            best_score = scores[best_mode]
            confidence = confidence * 0.8

        decision_data.append(
            {
                "scenario_id": i,
                "battery_level": round(battery_level, 1),
                "threat_level": round(threat_level, 3),
                "comm_strength": round(comm_strength, 3),
                "target_visibility": round(target_visibility, 3),
                "mission_priority": mission_priority,
                "distance_to_home_km": round(distance_to_home, 2),
                "weather": weather,
                "time_of_day": time_of_day,
                "wind_speed": round(wind_speed, 1),
                "reconnaissance_score": round(scores["reconnaissance"], 4),
                "surveillance_score": round(scores["surveillance"], 4),
                "pursuit_score": round(scores["pursuit"], 4),
                "loiter_score": round(scores["loiter"], 4),
                "return_home_score": round(scores["return_home"], 4),
                "engagement_score": round(scores["engagement"], 4),
                "selected_mode": best_mode,
                "selected_mode_score": round(best_score, 4),
                "decision_confidence": round(confidence, 3),
                "is_optimal_decision": int(confidence > 0.8),
                "meets_performance_spec": (
                    "YES" if (confidence > 0.7 and best_score > 0.3) else "NO"
                ),
            }
        )

    return pd.DataFrame(decision_data)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate mission decision data")
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("data/mission_decision_data.csv"),
        help="Output CSV path",
    )
    parser.add_argument("--scenarios", type=int, default=400, help="Number of scenarios")
    parser.add_argument("--seed", type=int, default=2026, help="Random seed")
    args = parser.parse_args()

    print("Generating Product 3 (Decision System) data...")
    df = generate_mission_decision_data(args.scenarios, args.seed)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    df.to_csv(args.output, index=False)

    print(f"Total scenarios: {len(df)}")
    print(f"Average confidence: {df['decision_confidence'].mean():.3f}")
    print(f"File saved: {args.output}")


if __name__ == "__main__":
    main()

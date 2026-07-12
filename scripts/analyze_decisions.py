#!/usr/bin/env python3
"""Decision log analysis dashboard (FR-3 / E2)."""

from __future__ import annotations

import argparse
import csv
import json
import statistics
import sys
from collections import Counter
from pathlib import Path


CSV_FIELDS = [
    "timestamp_ms",
    "battery",
    "threat",
    "comm",
    "visibility",
    "priority",
    "distance_km",
    "mode",
    "score",
    "confidence",
    "mode_changed",
    "low_confidence",
    "degraded_inputs",
    "emergency_forced",
]


def load_rows(path: Path) -> list[dict[str, str]]:
    if path.suffix.lower() == ".json":
        payload = json.loads(path.read_text(encoding="utf-8"))
        entries = payload.get("entries", [])
        rows = []
        for entry in entries:
            rows.append(
                {
                    "timestamp_ms": str(entry.get("timestamp_ms", 0)),
                    "battery": str(entry.get("battery", 0)),
                    "threat": str(entry.get("threat", 0)),
                    "comm": str(entry.get("comm", 0)),
                    "visibility": str(entry.get("visibility", 0)),
                    "priority": str(entry.get("priority", "medium")),
                    "distance_km": str(entry.get("distance_km", 0)),
                    "mode": str(entry.get("mode", "unknown")),
                    "score": str(entry.get("score", 0)),
                    "confidence": str(entry.get("confidence", 0)),
                    "mode_changed": "1" if entry.get("mode_changed") else "0",
                    "low_confidence": "1" if entry.get("low_confidence") else "0",
                    "degraded_inputs": "1" if entry.get("degraded_inputs") else "0",
                    "emergency_forced": "1" if entry.get("emergency_forced") else "0",
                }
            )
        return rows

    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def filter_rows(rows: list[dict[str, str]], args: argparse.Namespace) -> list[dict[str, str]]:
    filtered = []
    for row in rows:
        conf = float(row.get("confidence", 0))
        if conf < args.min_confidence or conf > args.max_confidence:
            continue
        if args.mode and row.get("mode") != args.mode:
            continue
        if args.low_confidence_only and row.get("low_confidence", "0") not in ("1", "true"):
            continue
        if args.emergency_only and row.get("emergency_forced", "0") not in ("1", "true"):
            continue
        if args.mode_changed_only and row.get("mode_changed", "0") not in ("1", "true"):
            continue
        filtered.append(row)
    return filtered


def ascii_bar(label: str, value: int, max_value: int, width: int = 30) -> str:
    if max_value <= 0:
        return f"  {label:<16} |"
    filled = int(width * value / max_value)
    return f"  {label:<16} | {'#' * filled} {value}"


def build_dashboard(rows: list[dict[str, str]]) -> str:
    if not rows:
        return "No entries to analyze."

    modes = Counter(row["mode"] for row in rows)
    confidences = [float(row["confidence"]) for row in rows]
    scores = [float(row["score"]) for row in rows]
    mode_changes = sum(1 for row in rows if row.get("mode_changed") in ("1", "true"))
    low_conf = sum(1 for row in rows if row.get("low_confidence") in ("1", "true"))
    emergencies = sum(1 for row in rows if row.get("emergency_forced") in ("1", "true"))
    degraded = sum(1 for row in rows if row.get("degraded_inputs") in ("1", "true"))

    lines = [
        "=" * 60,
        " Mili Decision Log Dashboard",
        "=" * 60,
        f"Entries analyzed : {len(rows)}",
        f"Mode changes     : {mode_changes}",
        f"Low confidence   : {low_conf}",
        f"Degraded inputs  : {degraded}",
        f"Emergency forced : {emergencies}",
        "",
        "Confidence",
        f"  min / avg / max: {min(confidences):.3f} / {statistics.mean(confidences):.3f} / {max(confidences):.3f}",
        f"  median         : {statistics.median(confidences):.3f}",
        "",
        "Score",
        f"  min / avg / max: {min(scores):.3f} / {statistics.mean(scores):.3f} / {max(scores):.3f}",
        "",
        "Mode distribution",
    ]

    max_mode = max(modes.values())
    for mode, count in modes.most_common():
        lines.append(ascii_bar(mode, count, max_mode))

    lines.append("")
    lines.append("Confidence histogram")
    bins = [0, 0, 0, 0, 0]
    for value in confidences:
        idx = min(4, int(value * 5))
        bins[idx] += 1
    labels = ["0.0-0.2", "0.2-0.4", "0.4-0.6", "0.6-0.8", "0.8-1.0"]
    max_bin = max(bins) if bins else 1
    for label, count in zip(labels, bins):
        lines.append(ascii_bar(label, count, max_bin))

    lines.append("=" * 60)
    return "\n".join(lines)


def export_filtered_csv(rows: list[dict[str, str]], path: Path) -> None:
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=CSV_FIELDS, extrasaction="ignore")
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def main() -> int:
    parser = argparse.ArgumentParser(description="Analyze and filter Mili decision logs")
    parser.add_argument("path", type=Path, help="CSV or JSON log export")
    parser.add_argument("--mode", type=str, default="", help="Filter by operational mode")
    parser.add_argument("--min-confidence", type=float, default=0.0)
    parser.add_argument("--max-confidence", type=float, default=1.0)
    parser.add_argument("--low-confidence-only", action="store_true")
    parser.add_argument("--emergency-only", action="store_true")
    parser.add_argument("--mode-changed-only", action="store_true")
    parser.add_argument("--export", type=Path, default=None, help="Write filtered CSV")
    parser.add_argument("--markdown", type=Path, default=None, help="Write markdown summary")
    args = parser.parse_args()

    if not args.path.exists():
        print(f"FAIL: file not found: {args.path}", file=sys.stderr)
        return 1

    rows = load_rows(args.path)
    filtered = filter_rows(rows, args)
    dashboard = build_dashboard(filtered)
    print(dashboard)

    if args.export:
        export_filtered_csv(filtered, args.export)
        print(f"Filtered CSV written to {args.export}")

    if args.markdown:
        args.markdown.write_text(f"# Decision Log Analysis\n\n```\n{dashboard}\n```\n", encoding="utf-8")
        print(f"Markdown report written to {args.markdown}")

    return 0 if filtered else 1


if __name__ == "__main__":
    raise SystemExit(main())

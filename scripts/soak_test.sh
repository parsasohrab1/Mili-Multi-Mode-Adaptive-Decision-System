#!/usr/bin/env bash
# G2 — Run continuous soak test (default: 72 min CI equivalent @ 10 Hz).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${MILI_BUILD_DIR:-$ROOT/build}"
CYCLES="${MILI_SOAK_CYCLES:-25920}"

cmake -B "$BUILD_DIR" -S "$ROOT" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"
MILI_SOAK_CYCLES="$CYCLES" ctest --test-dir "$BUILD_DIR" -C Release -R FieldQaTests --output-on-failure

echo "Soak complete ($CYCLES cycles). For full 72 h on-target: MILI_SOAK_CYCLES=2592000"

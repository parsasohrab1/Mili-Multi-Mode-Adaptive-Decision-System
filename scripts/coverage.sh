#!/usr/bin/env bash
# F4 — Generate gcov/lcov HTML coverage report (Linux / GCC or Clang).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${MILI_BUILD_DIR:-$ROOT/build}"

cmake -B "$BUILD_DIR" -S "$ROOT" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DMILI_ENABLE_COVERAGE=ON

cmake --build "$BUILD_DIR"
ctest --test-dir "$BUILD_DIR" --output-on-failure

INFO="$BUILD_DIR/coverage.info"
HTML="$BUILD_DIR/coverage-html"

lcov --directory "$BUILD_DIR" --capture --output-file "$INFO" --rc lcov_branch_coverage=0
lcov --remove "$INFO" '/usr/*' '*/tests/*' '*/platform/stm32/*' --output-file "$INFO"
genhtml "$INFO" --output-directory "$HTML" --legend --title "Mili Decision System"

echo "Coverage report: $HTML/index.html"
lcov --summary "$INFO"

#!/usr/bin/env bash
# F5 — clang-tidy + cppcheck (host sources).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${MILI_BUILD_DIR:-$ROOT/build}"

cmake -B "$BUILD_DIR" -S "$ROOT" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$BUILD_DIR" --target mili_decision

TIDY_ERRORS=0
if command -v run-clang-tidy >/dev/null 2>&1; then
    echo "=== clang-tidy ==="
    run-clang-tidy -p "$BUILD_DIR" -header-filter='include/mili/.*' \
        "$ROOT/src" "$ROOT/include" || TIDY_ERRORS=$?
elif command -v clang-tidy >/dev/null 2>&1; then
    echo "=== clang-tidy (per-file) ==="
    while IFS= read -r -d '' file; do
        clang-tidy -p "$BUILD_DIR" "$file" || TIDY_ERRORS=$?
    done < <(find "$ROOT/src" "$ROOT/include/mili" -name '*.cpp' -o -name '*.hpp' -print0)
else
    echo "WARN: clang-tidy not installed; skipping"
fi

CPPCHECK_ERRORS=0
if command -v cppcheck >/dev/null 2>&1; then
    echo "=== cppcheck ==="
    cppcheck --enable=warning,performance,portability \
        --std=c++17 --inline-suppr --error-exitcode=1 \
        -I "$ROOT/include" \
        "$ROOT/src" || CPPCHECK_ERRORS=$?
else
    echo "WARN: cppcheck not installed; skipping"
fi

if [[ $TIDY_ERRORS -ne 0 || $CPPCHECK_ERRORS -ne 0 ]]; then
    echo "Static analysis reported issues (tidy=$TIDY_ERRORS cppcheck=$CPPCHECK_ERRORS)"
    exit 1
fi

echo "Static analysis passed."

#!/usr/bin/env bash
# F6 — Cross-compile embedded firmware target (arm-none-eabi-gcc).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${MILI_STM32_BUILD_DIR:-$ROOT/build-stm32}"

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    echo "ERROR: arm-none-eabi-gcc not found. Install gcc-arm-none-eabi."
    exit 1
fi

cmake -B "$BUILD_DIR" -S "$ROOT" \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/platform/stm32/arm-gcc.cmake" \
    -DMILI_EMBEDDED=ON \
    -DMILI_BUILD_TESTS=OFF \
    -DMILI_BUILD_SIMULATOR=OFF \
    -DMILI_BUILD_EMBEDDED_HAL=ON

cmake --build "$BUILD_DIR"

FIRMWARE="$BUILD_DIR/platform/stm32/mili_stm32_firmware"
if [[ -f "$FIRMWARE" ]]; then
    arm-none-eabi-size "$FIRMWARE"
fi

echo "STM32 firmware built in $BUILD_DIR"

# Mili Decision System - Build Guide

## Prerequisites

- CMake 3.16+
- C++17 compiler (GCC, Clang, or MSVC)
- Python 3.10+ (for data generation)

See [INSTALL.md](INSTALL.md) for full install and troubleshooting.

## Linux / macOS Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
./build/mili_simulator
```

## Windows Build (MAX_PATH / F1)

Windows limits path length to ~260 characters (`MAX_PATH`). A deep clone path such as:

`C:\Users\...\Mili-Multi-Mode-Adaptive-Decision-System\Mili-Multi-Mode-Adaptive-Decision-System`

plus `build\` subdirs and MSVC object paths can exceed this limit and fail the build.

**Recommended:** use a short absolute build directory outside the repo:

```powershell
cmake -B C:\mili-build -S .
cmake --build C:\mili-build --config Release
ctest --test-dir C:\mili-build -C Release --output-on-failure
```

Helper script (defaults to `C:\mili-build`):

```powershell
.\scripts\build.ps1 -Test
```

Override with environment variable: `$env:MILI_BUILD_DIR = "D:\mili-build"`

Alternatives: `subst` drive mapping, or enable Windows long paths in Group Policy.

## Python Data Generation

```bash
pip install -r requirements.txt
python scripts/generate_mission_data.py --output data/mission_decision_data.csv
python scripts/validate_scoring.py
```

## Code Coverage (F4)

Linux with GCC/Clang and `lcov` installed:

```bash
bash scripts/coverage.sh
# Report: build/coverage-html/index.html
```

CMake option: `-DMILI_ENABLE_COVERAGE=ON` (Debug build).

## Static Analysis (F5)

```bash
bash scripts/static_analysis.sh
```

Requires `clang-tidy` and/or `cppcheck`. Uses `compile_commands.json` from CMake.

## ARM Cross-Compile (F6)

```bash
bash scripts/build_stm32.sh
```

Requires `arm-none-eabi-gcc`. See [platform/stm32/README.md](../platform/stm32/README.md).

## Phase Status

- [PHASE1.md](PHASE1.md) — core engine
- [PHASE2.md](PHASE2.md) — runtime weights ([API.md](API.md))
- [PHASE3.md](PHASE3.md) — product integration

## Project Structure

```
├── config/          # Runtime configuration (weights, hysteresis)
├── include/mili/    # Public C++ headers
├── src/             # Core implementation + simulator
├── scripts/         # Build, coverage, analysis utilities
├── tests/           # Unit tests
├── data/            # Generated datasets
├── platform/stm32/  # Embedded port + arm-gcc.cmake
└── .github/         # CI workflows
```

## Target Platform

Designed for STM32H7 (480 MHz, 512 KB RAM) with host-side simulation for development.

# Install & Quick Start

Guide for developers, integrators, and analysts setting up the Mili Decision System on a host PC.

## 1. Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| CMake | 3.16+ | Required |
| C++ compiler | C++17 | MSVC 2019+, GCC 9+, or Clang 10+ |
| Python | 3.10+ | Data generation and validation scripts |
| Git | any | Clone repository |

**Optional (embedded):**

| Tool | Purpose |
|------|---------|
| `gcc-arm-none-eabi` | F6 — cross-compile STM32 firmware |
| `lcov` + `genhtml` | F4 — HTML coverage reports (Linux) |
| `clang-tidy`, `cppcheck` | F5 — static analysis |

### Windows

- Install [Visual Studio](https://visualstudio.microsoft.com/) with “Desktop development with C++”, or MinGW-w64.
- Install [CMake](https://cmake.org/download/) and add to `PATH`.
- Install Python 3.11 from [python.org](https://www.python.org/) or Microsoft Store.

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y cmake g++ python3 python3-pip lcov clang-tidy cppcheck
pip install -r requirements.txt
```

### ARM toolchain (embedded)

```bash
# Ubuntu
sudo apt install gcc-arm-none-eabi

# Windows (Chocolatey)
choco install gcc-arm-embedded
```

---

## 2. Clone and Build

```bash
git clone <repo-url>
cd Mili-Multi-Mode-Adaptive-Decision-System
pip install -r requirements.txt
python scripts/generate_mission_data.py --scenarios 400
```

### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
./build/mili_simulator
```

### Windows (recommended short build path)

Deep clone paths can exceed Windows **MAX_PATH** (~260 characters) and break MSVC/CMake. Use a short build directory:

```powershell
cmake -B C:\mili-build -S .
cmake --build C:\mili-build --config Release
ctest --test-dir C:\mili-build -C Release --output-on-failure
C:\mili-build\Release\mili_simulator.exe
```

Or use the helper script:

```powershell
.\scripts\build.ps1 -Test -Simulator
```

Override build location: `$env:MILI_BUILD_DIR = "D:\mili-build"`

---

## 3. Configuration

Runtime weights live in `config/default_weights.json`. Load at startup:

```cpp
mili::DecisionEngine engine;
engine.load_weights("config/default_weights.json");
```

The file must include `"version": "1.0.0"` (semver). Same **major** version is accepted (e.g. `1.2.0` with runtime `1.0.0`). See [VERSIONING.md](VERSIONING.md).

Tune hysteresis, update rate, and log capacity in the same file.

---

## 4. Log Export and Analysis

```cpp
mili::LogExporter::write_csv(logger, "data/decision_export.csv");
```

```bash
python scripts/analyze_decisions.py data/decision_export.csv
```

See [REPORTING.md](REPORTING.md) and [ATP.md](ATP.md).

---

## 5. Embedded Cross-Compile (STM32H7)

```bash
./scripts/build_stm32.sh
```

```powershell
.\scripts\build_stm32.ps1
```

Requires `arm-none-eabi-gcc`. Output: `build-stm32/platform/stm32/mili_stm32_firmware`.

See [platform/stm32/README.md](../platform/stm32/README.md) and [EMBEDDED.md](EMBEDDED.md).

---

## 6. Quality Tooling

| Task | Command |
|------|---------|
| Coverage (Linux) | `bash scripts/coverage.sh` |
| Static analysis | `bash scripts/static_analysis.sh` |
| Weight validation | `python scripts/validate_weights.py` |
| Scoring validation | `python scripts/validate_scoring.py` |

---

## 7. Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| `error MSB3491` / path too long on Windows | MAX_PATH | Use `C:\mili-build` or `scripts\build.ps1` |
| `cmake` not found | CMake not in PATH | Install CMake, restart terminal |
| `cl` / `g++` not found | No C++ compiler | Install VS Build Tools or MinGW |
| Config load fails | Missing/wrong `version` | Add `"version": "1.0.0"`; see VERSIONING.md |
| `CsvValidationTests` fails | Missing data | Run `generate_mission_data.py` |
| `arm-none-eabi-gcc` not found | ARM toolchain missing | `apt install gcc-arm-none-eabi` |
| Python import errors | Deps not installed | `pip install -r requirements.txt` |
| Tests pass locally, fail in CI | Wrong working directory | Run ctest from repo root (CTest sets `WORKING_DIRECTORY`) |

### Windows long-path alternatives

1. **Short build dir** (recommended): `C:\mili-build`
2. **Subst drive**: `subst M: C:\long\path\to\repo` then build in `M:\build`
3. **Enable long paths**: Group Policy → “Enable Win32 long paths” (Windows 10 1607+)

---

## 8. Further Reading

- [BUILD.md](BUILD.md) — build details and project layout
- [DEPLOYMENT.md](DEPLOYMENT.md) — deployment and CI
- [docs/README.md](README.md) — full documentation index

# Deployment Guide

See [INSTALL.md](INSTALL.md) for install, configuration, and troubleshooting.

## Host Simulator

```bash
pip install -r requirements.txt
python scripts/generate_mission_data.py --scenarios 400

cmake -B C:/mili-build -S .
cmake --build C:/mili-build --config Release
ctest --test-dir C:/mili-build -C Release
C:/mili-build/Release/mili_simulator.exe
```

## Validation Scripts

```bash
python scripts/validate_scoring.py
python scripts/validate_weights.py
```

## Log Export

After simulation, export decision logs from application code:

```cpp
mili::LogExporter::write_csv(engine.logger(), "data/decision_export.csv");
mili::LogExporter::write_json(engine.logger(), "data/decision_export.json");
```

Analyze:

```bash
python scripts/analyze_decisions.py data/decision_export.csv
```

## CI

GitHub Actions runs on Ubuntu and Windows:

- CMake build + all tests (13 suites)
- Python scoring/weight validation
- Reporting tests + `analyze_decisions.py`
- Integrated simulator smoke test
- **Coverage** (Ubuntu): gcov/lcov HTML artifact
- **Static analysis** (Ubuntu): clang-tidy + cppcheck
- **ARM cross-compile** (Ubuntu): `mili_stm32_firmware` + size report

## Embedded

See [platform/stm32/README.md](../platform/stm32/README.md).

## Versioning

See [VERSIONING.md](VERSIONING.md).

- Config schema: `config/default_weights.json` (`"version": "1.0.0"`, semver)
- Protocol: `mili::protocol::kProtocolVersion`
- Contract: `mili::schema::kContractVersion`

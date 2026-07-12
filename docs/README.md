# Documentation Index

## Getting Started

| Document | Audience | Description |
|----------|----------|-------------|
| [INSTALL.md](INSTALL.md) | All users | Install, build, configure, troubleshoot |
| [BUILD.md](BUILD.md) | Developers | CMake, Windows MAX_PATH, project layout |
| [DEPLOYMENT.md](DEPLOYMENT.md) | DevOps | CI, simulator, log export |

## Product & Requirements

| Document | Description |
|----------|-------------|
| [../README.md](../README.md) | SRS / product specification (Persian) |
| [ROADMAP.md](ROADMAP.md) | Completion status and gaps |

## Feature Areas

| Sprint | Document |
|--------|----------|
| Algorithm A1–A6 | [ALGORITHM.md](ALGORITHM.md) |
| Embedded B1–B7 | [EMBEDDED.md](../platform/stm32/README.md) |
| Integration C1–C6 | [INTEGRATION.md](INTEGRATION.md) |
| Resilience D1–D7 | [RESILIENCE.md](RESILIENCE.md) |
| Reporting E1–E5 | [REPORTING.md](REPORTING.md), [ATP.md](ATP.md) |
| Field QA G1–G5 | [FIELD_QA.md](FIELD_QA.md), [ATP-FIELD.md](ATP-FIELD.md) |
| DevOps F1–F6 | [INSTALL.md](INSTALL.md), [VERSIONING.md](VERSIONING.md) |

## API & Phases

| Document | Description |
|----------|-------------|
| [API.md](API.md) | C++ public API reference |
| [PHASE1.md](PHASE1.md) | Core engine acceptance |
| [PHASE2.md](PHASE2.md) | Runtime weights |
| [PHASE3.md](PHASE3.md) | Product integration |

## QA & Analysis

| Document / Script | Description |
|-------------------|-------------|
| [ATP.md](ATP.md) | Acceptance test protocol |
| `scripts/analyze_decisions.py` | Decision log dashboard |
| `scripts/coverage.sh` | gcov/lcov report |
| `scripts/static_analysis.sh` | clang-tidy + cppcheck |

# Acceptance Test Protocol (ATP) — Product 3 Reporting (FR-3)

**Document:** ATP-MILI-P3-REPORTING v1.0  
**Scope:** Decision log export, embedded logger, analysis tooling (E1–E5)  
**Target:** STM32H7 @ 10 Hz decision loop; host simulator parity

---

## 1. Objectives

Verify that operational decisions can be recorded, exported (file or UART), filtered, and analyzed for mission review and regulatory traceability.

## 2. Preconditions

| ID | Requirement |
|----|-------------|
| P-1 | Firmware or host build completes without errors (`cmake --build`) |
| P-2 | All unit tests pass (`ctest -C Release`) |
| P-3 | Python 3.11+ with `requirements.txt` installed |
| P-4 | Mission dataset present: `data/mission_decision_data.csv` |

## 3. Functional Requirements

### FR-1 — Structured Export (E1)

| Test | Procedure | Pass Criteria |
|------|-----------|---------------|
| FR-1.1 CSV file | Run `ReportingTests`; inspect `test_export.csv` | Header + ≥1 data row; `mode` column present |
| FR-1.2 JSON file | Inspect `test_export.json` | `"version": 1` and `entries` array |
| FR-1.3 UART stream | `ReportingTests` UART capture | Header line + one CSV line per logged entry |
| FR-1.4 Schema | Compare columns to `docs/REPORTING.md` | All 14 fields present in CSV header |

### FR-2 — Embedded Logger (E4)

| Test | Procedure | Pass Criteria |
|------|-----------|---------------|
| FR-2.1 Capacity | `EmbeddedDecisionLogger::capacity()` | Equals `kEmbeddedLogCapacity` (32) |
| FR-2.2 No heap growth | Ring buffer stores N entries | `size() == N` for N ≤ capacity |
| FR-2.3 Export parity | `LogExporter::to_csv(ring_logger)` | Contains expected `mode` string |
| FR-2.4 UART drain | `UartLogExporter::drain` | `lines_sent` matches entry count |

### FR-3 — Analysis Dashboard (E2)

| Test | Procedure | Pass Criteria |
|------|-----------|---------------|
| FR-3.1 Load CSV | `python scripts/analyze_decisions.py test_export.csv` | Exit code 0; dashboard printed |
| FR-3.2 Load JSON | Same script on `test_export.json` | Exit code 0 |
| FR-3.3 Filter | `--mode surveillance --min-confidence 0.5` | Subset of rows analyzed |
| FR-3.4 Export | `--export filtered.csv` | File created with valid header |

### FR-4 — Log Filter API (E5)

| Test | Procedure | Pass Criteria |
|------|-----------|---------------|
| FR-4.1 Mode filter | `LogFilter::count` with `mode_filter` | Correct subset size |
| FR-4.2 Confidence | `low_confidence_only` | Matches entries with `confidence < 0.5` |
| FR-4.3 Mode change | `mode_changed_only` | Matches flagged transitions |

## 4. Automated Test Commands

```bash
# Configure & build (Windows example)
cmake -B C:/mili-build -S .
cmake --build C:/mili-build --config Release

# Reporting unit tests (generates test_export.csv / .json in repo root)
ctest --test-dir C:/mili-build -C Release -R ReportingTests --output-on-failure

# Python dashboard (from repo root)
python scripts/analyze_decisions.py test_export.csv
python scripts/analyze_decisions.py test_export.json --markdown build/atp_report.md
```

Linux: replace `C:/mili-build` with `build`.

## 5. Pass/Fail Matrix

| Area | Automated | Manual (optional) |
|------|-----------|-------------------|
| E1 File export | `ReportingTests` | SD-card copy on bench |
| E1 UART export | `ReportingTests` mock callback | Logic analyzer on UART3 |
| E2 Dashboard | `analyze_decisions.py` | Operator review of markdown |
| E4 Ring buffer | `ReportingTests` + `EmbeddedTests` | On-target RAM map check |
| E5 Filter | `ReportingTests` | Analyst workflow demo |

**Release gate:** All automated rows must pass on Ubuntu and Windows CI.

## 6. Traceability

| Sprint item | Module | Test |
|-------------|--------|------|
| E1 | `log_exporter`, `uart_log_exporter` | FR-1.x |
| E2 | `scripts/analyze_decisions.py` | FR-3.x |
| E3 | This document | Review sign-off |
| E4 | `ring_buffer_logger`, `EmbeddedDecisionLogger` | FR-2.x |
| E5 | `log_filter` | FR-4.x |

## 7. Sign-Off

| Role | Name | Date | Result |
|------|------|------|--------|
| Developer | | | ☐ Pass ☐ Fail |
| QA | | | ☐ Pass ☐ Fail |
| Systems | | | ☐ Pass ☐ Fail |

---

*Final acceptance report (ATP): the decision log output must be extractable (CSV/JSON/UART), analyzable with Python tools, and stored in embedded without dynamic allocation.*

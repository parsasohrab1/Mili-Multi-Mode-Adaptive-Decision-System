# Reporting Layer (E1–E5 / FR-3)

## E1 — CSV/JSON Export (UART or File)

`LogExporter` formats any `IDecisionLogger` implementation:

```cpp
mili::LogExporter::write_csv(logger, "data/decision_export.csv");
mili::LogExporter::write_json(logger, "data/decision_export.json");
```

Embedded UART drain (no heap):

```cpp
mili::UartLogExporter uart(uart_hal_write, nullptr);
uart.write_header();
uart.drain(logger);  // streams CSV lines over UART
```

Shared formatting lives in `log_format.hpp` (`snprintf`-based, embedded-safe).

## E2 — Python Analysis Dashboard

`scripts/analyze_decisions.py` reads CSV or JSON exports and prints an ASCII dashboard.

```bash
python scripts/analyze_decisions.py test_export.csv
python scripts/analyze_decisions.py test_export.json --mode surveillance --min-confidence 0.5
python scripts/analyze_decisions.py test_export.csv --export filtered.csv --markdown report.md
```

## E3 — Acceptance Test Protocol (ATP)

See `docs/ATP.md` for formal pass/fail criteria and release checklist.

## E4 — Embedded-Safe Ring Buffer Logger

`EmbeddedDecisionLogger` / `RingBufferLogger<kEmbeddedLogCapacity>` implements `IDecisionLogger` with fixed storage (no `std::vector`).

Export path is identical to host logging:

```cpp
mili::EmbeddedDecisionLogger logger;
// ... log decisions in 10 Hz loop ...
mili::LogExporter::to_csv(logger);      // host post-processing
mili::UartLogExporter::drain(logger);   // on-target UART dump
```

Capacity: `kEmbeddedLogCapacity` (32 entries).

## E5 — Log Filter and Search

`LogFilter` + `LogQuery` filter in-memory logs before export:

```cpp
mili::LogQuery query{};
query.mode_filter = mili::OperationalMode::Loiter;
query.low_confidence_only = true;
const auto matches = mili::LogFilter::apply(logger, query);
```

Filters: mode, confidence range, timestamp window, low confidence, degraded inputs, emergency, mode-changed.

## Tests

```bash
ctest -C Release -R ReportingTests --output-on-failure
```

Covers file export, ring-buffer + UART drain, and `LogFilter` queries.

## Data Schema

CSV columns: `timestamp_ms`, `battery`, `threat`, `comm`, `visibility`, `priority`, `distance_km`, `mode`, `score`, `confidence`, `mode_changed`, `low_confidence`, `degraded_inputs`, `emergency_forced`.

JSON envelope: `{ "version": 1, "entries": [ ... ] }`.

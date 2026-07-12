# Mili Decision System — Internal API

## Overview

The decision stack is composed of:

- `WeightManager` — runtime weight configuration
- `ModeScorer` — multi-criteria mode scoring
- `HysteresisController` — anti-oscillation gate
- `DecisionLogger` — rolling decision history
- `DecisionEngine` — orchestration facade
- `SensorAdapter` — merges Product 1/2, environmental, and operator inputs
- `DecisionLoop` — 10 Hz decision tick orchestration

All public types live in namespace `mili`.

---

## WeightManager

Loads and updates scoring weights without restarting the engine.

### Construction

```cpp
mili::WeightManager manager;
```

Starts with compiled-in defaults from `make_default_weight_config()`.

### Load / Reload

```cpp
bool load_from_file(const std::string& path);
bool load_from_string(const std::string& json);
bool reload();
```

- `load_from_file` reads JSON from disk and applies it.
- `load_from_string` applies JSON from memory.
- `reload` re-reads the last successful file path.

On parse failure, the manager **falls back to defaults** and returns `false`.

### Runtime Weight API

```cpp
bool set_mode_factor(OperationalMode mode, const char* factor_name, float value);
bool set_priority_multiplier(MissionPriority priority, OperationalMode mode, float multiplier);
bool set_environmental_multiplier(const char* condition, OperationalMode mode, float multiplier);
```

Example — increase engagement visibility weight at runtime:

```cpp
manager.set_mode_factor(
    mili::OperationalMode::Engagement,
    "target_visibility",
    0.8f);
```

Supported engagement factors:

| Factor | Description |
|--------|-------------|
| `target_visibility` | Weight for target visibility |
| `threat_threshold` | Weight for high-threat indicator |
| `battery_threshold` | Weight for sufficient-battery indicator |
| `priority_boost` | Weight for critical-priority boost |
| `distance_penalty` | Per-km distance penalty multiplier |

### Introspection

```cpp
const WeightConfig& config() const;
WeightConfig& config();
bool using_defaults() const;
const std::string& last_loaded_path() const;
std::size_t reload_count() const;
```

---

## ModeScorer

Computes per-mode scores using weights from an attached `WeightManager`.

```cpp
mili::WeightManager weights;
mili::ModeScorer scorer(&weights);

auto scores = scorer.compute_scores(sensor_input);
auto best = mili::select_best_mode(scores);
```

If constructed without a manager pointer, built-in defaults are used.

Scoring formula per mode:

```
Score(mode) = Σ(w_i × normalized_factor_i) × priority_modifiers × environmental_modifiers
```

---

## DecisionEngine

### Default construction

```cpp
mili::DecisionEngine engine(0);  // timestamp_ms = 0
auto result = engine.evaluate(input, timestamp_ms);
```

### Dependency injection

```cpp
mili::EngineComponents components;
components.weight_manager = &custom_weights;
components.scorer = &custom_scorer;
components.hysteresis = &custom_hysteresis;
components.logger = &custom_logger;

mili::DecisionEngine engine(components, 0);
```

Any `nullptr` component is replaced by an internally owned default.

### Configuration API

```cpp
bool load_config(const std::string& path);
bool reload_config();
bool set_mode_weight(OperationalMode mode, const char* factor_name, float value);
```

`load_config` / `reload_config` also apply `hysteresis_seconds` from the JSON file.

### Decision output

```cpp
struct DecisionResult {
    OperationalMode selected_mode;
    float selected_score;
    float confidence;
    ModeParameters control_parameters;
    std::array<ModeScore, 6> all_scores;
    bool mode_changed;
    std::uint64_t timestamp_ms;
};
```

---

## Config File Format

Default path: `config/default_weights.json`

```json
{
  "modes": {
    "engagement": {
      "target_visibility": 0.5,
      "threat_threshold": 0.3,
      "battery_threshold": 0.1,
      "distance_penalty": 0.02,
      "priority_boost": 0.2
    }
  },
  "priority_multipliers": { ... },
  "environmental": { ... },
  "hysteresis_seconds": 5.0,
  "update_rate_hz": 10,
  "max_decision_log_entries": 1000
}
```

---

## Typical Integration Flow

```cpp
mili::DecisionEngine engine;

engine.load_config("config/default_weights.json");

mili::SensorInput input{};
input.battery_level = 72.0f;
input.threat_level = 0.55f;
input.target_visibility = 0.85f;

// Runtime tuning without restart
engine.set_mode_weight(
    mili::OperationalMode::Engagement,
    "target_visibility",
    0.8f);

const auto result = engine.evaluate(input, now_ms);
```

---

## Error Handling

| Operation | On failure |
|-----------|------------|
| `load_from_file` | Returns `false`, restores defaults |
| `load_from_string` | Returns `false`, restores defaults |
| `set_mode_factor` | Returns `false`, config unchanged |
| `reload` | Returns `false` if no prior path |

---

## Thread Safety

Phase 2 components are **not thread-safe**. External synchronization is required if `evaluate()` and config reload run on different threads.

---

## Related Docs

- [BUILD.md](BUILD.md) — build instructions
- [PHASE1.md](PHASE1.md) — core engine acceptance
- [PHASE2.md](PHASE2.md) — runtime configuration acceptance
- [PHASE3.md](PHASE3.md) — product integration layer

---

## SensorAdapter (Phase 3)

Merges Product 1, Product 2, environmental sensors, and operator commands into `SensorInput`.

```cpp
mili::SensorAdapter adapter;
adapter.ingest_product1(product1_data, now_ms);
adapter.ingest_product2(product2_data, now_ms);
adapter.ingest_environmental(env_data, now_ms);
adapter.ingest_operator(operator_command, now_ms);

const auto input = adapter.to_sensor_input(now_ms);
```

Stale data (> 2 s default) continues using the last valid sample.

## DecisionLoop (Phase 3)

Runs the engine at a fixed rate (default 10 Hz):

```cpp
mili::DecisionLoop loop(engine, adapter, 10.0f);
const auto result = loop.tick(timestamp_ms);
const auto& stats = loop.stats();  // jitter metrics
```

# Versioning Policy (F3)

Semantic versioning for configuration, protocol, and inter-product contracts.

## Version Matrix

| Artifact | Field / constant | Current | Policy |
|----------|------------------|---------|--------|
| Weight config JSON | `"version"` | `1.0.0` | Semver; same major = compatible |
| C++ constant | `mili::kWeightConfigSchemaVersion` | `1.0.0` | Must match supported config major |
| Data contract | `mili::schema::kContractVersion` | `1` | Exact match only |
| CAN protocol | `mili::protocol::kProtocolVersion` | `1` | Exact match only |
| Log JSON export | `"version"` | `1` | Integer envelope version |
| CMake project | `project(VERSION …)` | `1.0.0` | Release tagging |

## Weight Config (semver)

`config/default_weights.json`:

```json
{
  "version": "1.0.0",
  ...
}
```

`WeightManager::load_from_file` rejects configs when:

- `"version"` is missing
- Semver parse fails
- **Major version** differs from `kWeightConfigSchemaVersion`

Compatible examples (runtime `1.0.0`):

- `1.0.0` ✅
- `1.1.0` ✅
- `2.0.0` ❌

API:

```cpp
mili::is_compatible_weight_config("1.2.3");  // true if major == 1
mili::parse_semver("1.0.0", semver);
```

## Bumping Versions

| Change type | Bump | Example |
|-------------|------|---------|
| New optional JSON field, same semantics | PATCH | `1.0.0` → `1.0.1` |
| New mode weights block, backward compatible | MINOR | `1.0.0` → `1.1.0` |
| Removed/renamed required fields | MAJOR | `1.x` → `2.0.0` |
| Contract header layout change | `kContractVersion++` | Breaking for Product 1/2 |
| New CAN message layout | `kProtocolVersion++` | Breaking for bus peers |

When bumping config major:

1. Update `kWeightConfigSchemaVersion` in `include/mili/config_version.hpp`
2. Provide migration notes in commit / release notes
3. Add fixture `tests/fixtures/wrong_version_config.json` pattern for CI

## Tests

```bash
ctest -C Release -R WeightManagerTests --output-on-failure
```

Validates default load, invalid JSON fallback, and major-version rejection.

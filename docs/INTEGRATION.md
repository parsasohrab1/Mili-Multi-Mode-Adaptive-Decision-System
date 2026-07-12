# Product Integration (C1–C6)

## C1 — Product 1 Power Management API

`PowerManagementClient` (`include/mili/integration/product1_api.hpp`) implements `IProduct1Client`:

| Method | Description |
|--------|-------------|
| `on_status_frame()` | CAN 0x301 + `ContractHeader` |
| `battery_percent()` | State of charge |
| `power_draw_watts()` | Instantaneous draw |
| `remaining_flight_time_min()` | Estimated endurance |
| `power_budget_watts()` | Available power budget |
| `low_power_warning()` | Critical battery flag |

## C2 — Product 2 VIO / Navigation API

`VioNavigationClient` (`include/mili/integration/product2_api.hpp`) implements `IProduct2Client`:

| Method | Description |
|--------|-------------|
| `on_nav_frame()` | CAN 0x302 navigation |
| `on_vio_frame()` | CAN 0x304 VIO extension |
| `distance_to_home_km()` | Home range |
| `heading_deg()` | Heading |
| `vio_confidence()` | VIO quality 0–1 |

## C3 — Shared Data Contract

`include/mili/schema/data_contract.hpp`:

```cpp
struct ContractHeader {
    uint8_t schema_version;   // kContractVersion = 1
    uint8_t source_id;        // Product1Power, Product2Vio, ...
    uint16_t sequence;
    uint32_t source_timestamp_ms;
};
```

All product APIs validate schema version and source ID before accepting data.

## C4 — Control System Output

`ControlSystemPublisher` sends **two** CAN frames per decision:

| CAN ID | Payload | Contents |
|--------|---------|----------|
| 0x310 | `ModeCommandPayload` | Mode + primary factors |
| 0x313 | `ControlParametersPayload` | **loiter_radius**, **return_urgency**, all mode parameters |

## C5 — Timestamp Synchronization

`TimestampSynchronizer` tracks per-source clock offset and drift:

- `observe(source, source_ts, local_rx_ts)`
- `aligned_timestamp()` — maps source time to local timeline
- `is_synchronized()` — verifies P1+P2 within skew budget

Integrated into `PowerManagementClient` and `VioNavigationClient`.

## C6 — HIL Testing

`mili::hil::HilRunner` replays CAN timelines from `data/hil_scenario.csv` through `HardwareMock`:

```
HardwareMock → PowerManagementClient + VioNavigationClient
            → IntegrationHub → DecisionEngine
            → ControlSystemPublisher
```

Run: `ctest -C Release -R HilTests`

## Simulator

`mili_simulator` now uses real Product APIs (mock hardware feeds CAN-shaped frames).

#pragma once

#include "mili/integration/interfaces.hpp"
#include "mili/integration/timestamp_sync.hpp"
#include "mili/product_data.hpp"
#include "mili/protocol/messages.hpp"
#include "mili/schema/data_contract.hpp"

#include <cstdint>

namespace mili {

// Product 1 — Power Management API (replaces mock for production wiring).
class PowerManagementClient : public IProduct1Client {
public:
    explicit PowerManagementClient(TimestampSynchronizer* sync = nullptr);

    void on_status_frame(
        const protocol::Product1CanPayload& payload,
        const schema::ContractHeader& header,
        std::uint64_t rx_timestamp_ms);

    void on_status_frame_legacy(const protocol::Product1CanPayload& payload, std::uint64_t rx_timestamp_ms);

    float battery_percent() const;
    float power_draw_watts() const;
    float remaining_flight_time_min() const;
    float power_budget_watts() const;
    bool low_power_warning() const;

    bool poll(Product1Data& out, std::uint64_t now_ms) override;
    bool is_connected() const override;

private:
    TimestampSynchronizer* sync_;
    Product1Data latest_{};
};

}  // namespace mili

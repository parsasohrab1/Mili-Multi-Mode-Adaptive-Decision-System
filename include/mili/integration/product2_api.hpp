#pragma once

#include "mili/integration/interfaces.hpp"
#include "mili/integration/timestamp_sync.hpp"
#include "mili/product_data.hpp"
#include "mili/protocol/messages.hpp"
#include "mili/schema/data_contract.hpp"

#include <cstdint>

namespace mili {

// Product 2 — VIO / Navigation API (replaces mock for production wiring).
class VioNavigationClient : public IProduct2Client {
public:
    explicit VioNavigationClient(TimestampSynchronizer* sync = nullptr);

    void on_nav_frame(const protocol::Product2CanPayload& payload, std::uint64_t rx_timestamp_ms);
    void on_vio_frame(
        const protocol::Product2VioExtPayload& vio_payload,
        const schema::ContractHeader& header,
        std::uint64_t rx_timestamp_ms);

    float distance_to_home_km() const;
    float velocity_mps() const;
    float heading_deg() const;
    float vio_confidence() const;

    bool poll(Product2Data& out, std::uint64_t now_ms) override;
    bool is_connected() const override;

private:
    TimestampSynchronizer* sync_;
    Product2Data latest_{};
    protocol::Product2CanPayload pending_nav_{};
    bool has_nav_ = false;
};

}  // namespace mili

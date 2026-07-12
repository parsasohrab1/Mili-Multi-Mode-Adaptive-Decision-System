#include "mili/integration/product1_api.hpp"

namespace mili {

PowerManagementClient::PowerManagementClient(TimestampSynchronizer* sync)
    : sync_(sync) {}

void PowerManagementClient::on_status_frame(
    const protocol::Product1CanPayload& payload,
    const schema::ContractHeader& header,
    std::uint64_t rx_timestamp_ms) {
    latest_ = protocol::decode_product1(payload, header, rx_timestamp_ms);
    if (sync_ != nullptr && latest_.valid) {
        sync_->observe(schema::DataSourceId::Product1Power, header.source_timestamp_ms, rx_timestamp_ms);
        latest_.timestamp_ms = sync_->aligned_timestamp(
            schema::DataSourceId::Product1Power, header.source_timestamp_ms);
    }
}

void PowerManagementClient::on_status_frame_legacy(
    const protocol::Product1CanPayload& payload, std::uint64_t rx_timestamp_ms) {
    latest_ = protocol::decode_product1(payload, rx_timestamp_ms);
}

float PowerManagementClient::battery_percent() const {
    return latest_.battery_level_percent;
}

float PowerManagementClient::power_draw_watts() const {
    return latest_.power_draw_watts;
}

float PowerManagementClient::remaining_flight_time_min() const {
    return latest_.remaining_flight_time_min;
}

float PowerManagementClient::power_budget_watts() const {
    return latest_.power_budget_watts;
}

bool PowerManagementClient::low_power_warning() const {
    return latest_.low_power_warning;
}

bool PowerManagementClient::poll(Product1Data& out, std::uint64_t now_ms) {
    if (!latest_.valid) {
        return false;
    }
    if (now_ms > latest_.timestamp_ms + kDefaultStaleThresholdMs) {
        return false;
    }
    out = latest_;
    return true;
}

bool PowerManagementClient::is_connected() const {
    return latest_.valid;
}

}  // namespace mili

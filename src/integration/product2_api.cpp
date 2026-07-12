#include "mili/integration/product2_api.hpp"

namespace mili {

VioNavigationClient::VioNavigationClient(TimestampSynchronizer* sync)
    : sync_(sync) {}

void VioNavigationClient::on_nav_frame(
    const protocol::Product2CanPayload& payload, std::uint64_t rx_timestamp_ms) {
    pending_nav_ = payload;
    has_nav_ = true;
    latest_ = protocol::decode_product2(payload, rx_timestamp_ms);
}

void VioNavigationClient::on_vio_frame(
    const protocol::Product2VioExtPayload& vio_payload,
    const schema::ContractHeader& header,
    std::uint64_t rx_timestamp_ms) {
    if (!has_nav_) {
        return;
    }
    latest_ = protocol::decode_product2(pending_nav_, vio_payload, header, rx_timestamp_ms);
    if (sync_ != nullptr && latest_.valid) {
        sync_->observe(schema::DataSourceId::Product2Vio, header.source_timestamp_ms, rx_timestamp_ms);
        latest_.timestamp_ms = sync_->aligned_timestamp(
            schema::DataSourceId::Product2Vio, header.source_timestamp_ms);
    }
}

float VioNavigationClient::distance_to_home_km() const {
    return latest_.distance_to_home_km;
}

float VioNavigationClient::velocity_mps() const {
    return latest_.velocity_mps;
}

float VioNavigationClient::heading_deg() const {
    return latest_.heading_deg;
}

float VioNavigationClient::vio_confidence() const {
    return latest_.vio_confidence;
}

bool VioNavigationClient::poll(Product2Data& out, std::uint64_t now_ms) {
    if (!latest_.valid) {
        return false;
    }
    if (now_ms > latest_.timestamp_ms + kDefaultStaleThresholdMs) {
        return false;
    }
    out = latest_;
    return true;
}

bool VioNavigationClient::is_connected() const {
    return latest_.valid;
}

}  // namespace mili

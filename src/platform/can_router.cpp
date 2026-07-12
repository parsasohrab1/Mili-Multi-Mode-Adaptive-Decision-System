#include "mili/platform/can_router.hpp"

#include "mili/integration/integration_hub.hpp"
#include "mili/product_data.hpp"
#include "mili/sensor_adapter.hpp"

#include <cstring>

namespace mili {

bool CanRouter::send_frame(
    CanSendCallback callback, void* user_data, std::uint16_t can_id, const void* data, std::size_t len) {
    if (callback == nullptr) {
        return false;
    }
    return callback(can_id, data, len, user_data);
}

CanRouter::CanRouter(
    CanProduct1Client& product1,
    CanProduct2Client& product2,
    CanOperatorClient& operator_client,
    SensorAdapter& adapter)
    : product1_(product1)
    , product2_(product2)
    , operator_(operator_client)
    , adapter_(adapter) {}

void CanRouter::route_frame(
    std::uint16_t can_id, const std::uint8_t* data, std::size_t len, std::uint64_t timestamp_ms) {
    if (data == nullptr) {
        return;
    }

    if (can_id == protocol::kCanIdProduct1Status && len >= sizeof(protocol::Product1CanPayload)) {
        protocol::Product1CanPayload payload{};
        std::memcpy(&payload, data, sizeof(payload));
        product1_.on_frame(payload, timestamp_ms);
        return;
    }

    if (can_id == protocol::kCanIdProduct2Nav && len >= sizeof(protocol::Product2CanPayload)) {
        protocol::Product2CanPayload payload{};
        std::memcpy(&payload, data, sizeof(payload));
        product2_.on_frame(payload, timestamp_ms);
        return;
    }

    if (can_id == protocol::kCanIdOperatorCmd && len >= sizeof(protocol::OperatorCanPayload)) {
        protocol::OperatorCanPayload payload{};
        std::memcpy(&payload, data, sizeof(payload));
        operator_.on_frame(payload, timestamp_ms);
        return;
    }

    (void)adapter_;
}

void CanOperatorClient::on_frame(const protocol::OperatorCanPayload& payload, std::uint64_t timestamp_ms) {
    latest_ = protocol::decode_operator(payload, timestamp_ms);
}

bool CanOperatorClient::poll(OperatorCommand& out, std::uint64_t now_ms) {
    if (!latest_.valid) {
        return false;
    }
    if (now_ms > latest_.timestamp_ms + kDefaultStaleThresholdMs) {
        return false;
    }
    out = latest_;
    return true;
}

bool CanOperatorClient::is_connected() const {
    return latest_.valid;
}

CanReportPublisher::CanReportPublisher(CanSendCallback send_frame, void* user_data)
    : send_frame_(send_frame)
    , user_data_(user_data) {}

bool CanReportPublisher::publish(const DecisionResult& result, std::uint16_t evaluate_time_us) {
    last_ = protocol::encode_decision_report(result, evaluate_time_us);
    return CanRouter::send_frame(
        send_frame_, user_data_, protocol::kCanIdDecisionReport, &last_, sizeof(last_));
}

const protocol::DecisionReportPayload& CanReportPublisher::last_payload() const {
    return last_;
}

}  // namespace mili

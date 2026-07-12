#pragma once

#include "mili/protocol/messages.hpp"

#include <cstddef>
#include <cstdint>

namespace mili {

class CanProduct1Client;
class CanProduct2Client;
class CanOperatorClient;
class IntegrationHub;
class SensorAdapter;

using CanSendCallback = bool (*)(std::uint16_t can_id, const void* data, std::size_t len, void* user_data);

class CanRouter {
public:
    CanRouter(
        CanProduct1Client& product1,
        CanProduct2Client& product2,
        CanOperatorClient& operator_client,
        SensorAdapter& adapter);

    void route_frame(std::uint16_t can_id, const std::uint8_t* data, std::size_t len, std::uint64_t timestamp_ms);

    static bool send_frame(CanSendCallback callback, void* user_data, std::uint16_t can_id, const void* data, std::size_t len);

private:
    CanProduct1Client& product1_;
    CanProduct2Client& product2_;
    CanOperatorClient& operator_;
    SensorAdapter& adapter_;
};

class CanOperatorClient {
public:
    void on_frame(const protocol::OperatorCanPayload& payload, std::uint64_t timestamp_ms);
    bool poll(OperatorCommand& out, std::uint64_t now_ms);
    bool is_connected() const;

private:
    OperatorCommand latest_{};
};

class CanReportPublisher {
public:
    explicit CanReportPublisher(CanSendCallback send_frame, void* user_data = nullptr);

    bool publish(const DecisionResult& result, std::uint16_t evaluate_time_us);

    const protocol::DecisionReportPayload& last_payload() const;

private:
    CanSendCallback send_frame_;
    void* user_data_;
    protocol::DecisionReportPayload last_{};
};

}  // namespace mili

#pragma once

#include "mili/data_quality_monitor.hpp"
#include "mili/decision_engine.hpp"
#include "mili/health_monitor.hpp"
#include "mili/integration/interfaces.hpp"
#include "mili/protocol/messages.hpp"
#include "mili/sensor_adapter.hpp"

#include <cstdint>
#include <functional>

namespace mili {

using CanFrameHandler = std::function<bool(std::uint16_t can_id, const std::uint8_t* data, std::size_t len)>;

class IntegrationHub {
public:
    IntegrationHub(SensorAdapter& adapter, IControlPublisher& publisher);

    void set_product1_client(IProduct1Client* client);
    void set_product2_client(IProduct2Client* client);

    void poll_inputs(std::uint64_t now_ms);
    DecisionResult run_cycle(DecisionEngine& engine, std::uint64_t now_ms);

    bool product1_connected() const;
    bool product2_connected() const;
    const DataQualityStatus& last_quality_status() const;
    const SystemHealthMonitor& health_monitor() const;
    bool is_healthy() const;

private:
    SensorAdapter& adapter_;
    IControlPublisher& publisher_;
    DataQualityMonitor quality_monitor_;
    SystemHealthMonitor health_monitor_;
    DataQualityStatus last_quality_{};
    IProduct1Client* product1_ = nullptr;
    IProduct2Client* product2_ = nullptr;

    Product1Data last_p1_{};
    Product2Data last_p2_{};
    EnvironmentalData last_env_{};
    OperatorCommand last_op_{};
};

class CanProduct1Client : public IProduct1Client {
public:
    void on_frame(const protocol::Product1CanPayload& payload, std::uint64_t timestamp_ms);
    bool poll(Product1Data& out, std::uint64_t now_ms) override;
    bool is_connected() const override;

private:
    Product1Data latest_{};
};

class CanProduct2Client : public IProduct2Client {
public:
    void on_frame(const protocol::Product2CanPayload& payload, std::uint64_t timestamp_ms);
    bool poll(Product2Data& out, std::uint64_t now_ms) override;
    bool is_connected() const override;

private:
    Product2Data latest_{};
};

class CanControlPublisher : public IControlPublisher {
public:
    using SendFrameFn = std::function<bool(std::uint16_t can_id, const void* data, std::size_t len)>;

    explicit CanControlPublisher(SendFrameFn send_frame);

    bool publish(const DecisionResult& result) override;
    const protocol::ModeCommandPayload& last_payload() const;

private:
    SendFrameFn send_frame_;
    protocol::ModeCommandPayload last_{};
};

}  // namespace mili

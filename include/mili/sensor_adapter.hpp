#pragma once

#include "mili/operator_command_handler.hpp"
#include "mili/product1_adapter.hpp"
#include "mili/product2_adapter.hpp"
#include "mili/product_data.hpp"
#include "mili/types.hpp"

namespace mili {

class SensorAdapter {
public:
    explicit SensorAdapter(std::uint64_t stale_threshold_ms = kDefaultStaleThresholdMs);

    void ingest_product1(const Product1Data& data, std::uint64_t now_ms);
    void ingest_product2(const Product2Data& data, std::uint64_t now_ms);
    void ingest_environmental(const EnvironmentalData& data, std::uint64_t now_ms);
    void ingest_operator(const OperatorCommand& command, std::uint64_t now_ms);

    SensorInput to_sensor_input(std::uint64_t now_ms) const;

    bool product1_is_stale(std::uint64_t now_ms) const;
    bool product2_is_stale(std::uint64_t now_ms) const;
    bool environmental_is_stale(std::uint64_t now_ms) const;

    EnvironmentalData last_environmental() const;
    OperatorCommand last_operator() const;

private:
    std::uint64_t stale_threshold_ms_;
    Product1Adapter product1_;
    Product2Adapter product2_;
    EnvironmentalData environmental_{};
    OperatorCommandHandler operator_handler_;
    MissionPriority default_priority_ = MissionPriority::Medium;

    static float clamp_battery(float value);
};

}  // namespace mili

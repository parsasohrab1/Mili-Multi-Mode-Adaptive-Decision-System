#pragma once

#include "mili/decision_engine.hpp"
#include "mili/embedded_config.hpp"
#include "mili/integration/integration_hub.hpp"
#include "mili/platform/benchmark.hpp"
#include "mili/platform/can_router.hpp"
#include "mili/sensor_adapter.hpp"

#include <cstdint>

namespace mili::embedded {

struct AppContext {
    SensorAdapter adapter;
    CanProduct1Client product1;
    CanProduct2Client product2;
    CanOperatorClient operator_client;
    CanRouter router;
    CanControlPublisher* control_publisher;
    CanReportPublisher* report_publisher;
    IntegrationHub* hub;
    DecisionEngine* engine;
    std::uint64_t tick_ms;
    std::uint16_t last_evaluate_us;

    AppContext(
        CanControlPublisher& control_pub,
        CanReportPublisher& report_pub,
        IntegrationHub& hub_ref,
        DecisionEngine& engine_ref);
};

void app_init(AppContext& ctx);
void app_on_can_frame(AppContext& ctx, std::uint16_t can_id, const std::uint8_t* data, std::size_t len);
DecisionResult app_tick(AppContext& ctx, std::uint64_t now_ms);
BenchmarkResult app_run_benchmark(AppContext& ctx, std::uint32_t iterations);

}  // namespace mili::embedded

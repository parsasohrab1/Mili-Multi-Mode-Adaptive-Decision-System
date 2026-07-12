#include "mili_app.hpp"

#include "mili/embedded_config.hpp"
#include "mili/protocol/messages.hpp"

#include <chrono>
#include <cstring>

namespace mili::embedded {

AppContext::AppContext(
    CanControlPublisher& control_pub,
    CanReportPublisher& report_pub,
    IntegrationHub& hub_ref,
    DecisionEngine& engine_ref)
    : router(product1, product2, operator_client, adapter)
    , control_publisher(&control_pub)
    , report_publisher(&report_pub)
    , hub(&hub_ref)
    , engine(&engine_ref)
    , tick_ms(0)
    , last_evaluate_us(0) {}

void app_init(AppContext& ctx) {
    ctx.hub->set_product1_client(&ctx.product1);
    ctx.hub->set_product2_client(&ctx.product2);
    ctx.engine->set_noise_enabled(true);
    ctx.engine->set_mcdm_enabled(true);
}

void app_on_can_frame(AppContext& ctx, std::uint16_t can_id, const std::uint8_t* data, std::size_t len) {
    ctx.router.route_frame(can_id, data, len, ctx.tick_ms);

    if (can_id == protocol::kCanIdConfigUpdate && len >= sizeof(protocol::ConfigUpdatePayload)) {
        protocol::ConfigUpdatePayload payload{};
        std::memcpy(&payload, data, sizeof(payload));
        protocol::ConfigCommand command{};
        OperationalMode mode{};
        std::uint8_t factor_id = 0;
        float value = 0.0f;
        if (!protocol::decode_config_update(payload, command, mode, factor_id, value)) {
            return;
        }
        switch (command) {
            case protocol::ConfigCommand::ReloadDefaults:
                ctx.engine->reload_config();
                break;
            case protocol::ConfigCommand::SetHysteresis:
                ctx.engine->set_hysteresis_seconds(value * 10.0f);
                break;
            case protocol::ConfigCommand::SetModeFactor:
                ctx.engine->set_mode_weight(mode, "visibility", value);
                break;
            default:
                break;
        }
    }

    OperatorCommand op{};
    if (ctx.operator_client.poll(op, ctx.tick_ms)) {
        ctx.adapter.ingest_operator(op, ctx.tick_ms);
    }
}

DecisionResult app_tick(AppContext& ctx, std::uint64_t now_ms) {
    ctx.tick_ms = now_ms;

    const auto start = std::chrono::steady_clock::now();
    const auto result = ctx.hub->run_cycle(*ctx.engine, now_ms);
    const auto end = std::chrono::steady_clock::now();

    const auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    ctx.last_evaluate_us = static_cast<std::uint16_t>(
        elapsed_us > 65535 ? 65535 : elapsed_us);

    if (ctx.report_publisher != nullptr) {
        ctx.report_publisher->publish(result, ctx.last_evaluate_us);
    }

    return result;
}

BenchmarkResult app_run_benchmark(AppContext& ctx, std::uint32_t iterations) {
    SensorInput input{};
    input.battery_level = 75.0f;
    input.threat_level = 0.2f;
    input.target_visibility = 0.85f;
    input.comm_strength = 0.8f;
    return benchmark_evaluate(*ctx.engine, input, iterations);
}

}  // namespace mili::embedded

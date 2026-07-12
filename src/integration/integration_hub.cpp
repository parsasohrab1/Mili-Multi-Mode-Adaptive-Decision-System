#include "mili/integration/integration_hub.hpp"

#include "mili/mode_parameters.hpp"
#include "mili/sensor_imputer.hpp"

namespace mili {

IntegrationHub::IntegrationHub(SensorAdapter& adapter, IControlPublisher& publisher)
    : adapter_(adapter)
    , publisher_(publisher)
    , health_monitor_(100, 500) {}

void IntegrationHub::set_product1_client(IProduct1Client* client) {
    product1_ = client;
}

void IntegrationHub::set_product2_client(IProduct2Client* client) {
    product2_ = client;
}

void IntegrationHub::poll_inputs(std::uint64_t now_ms) {
    if (product1_ != nullptr) {
        Product1Data data{};
        if (product1_->poll(data, now_ms)) {
            last_p1_ = data;
            adapter_.ingest_product1(data, now_ms);
        }
    }

    if (product2_ != nullptr) {
        Product2Data data{};
        if (product2_->poll(data, now_ms)) {
            last_p2_ = data;
            adapter_.ingest_product2(data, now_ms);
        }
    }

    last_env_ = adapter_.last_environmental();
    last_op_ = adapter_.last_operator();
    (void)now_ms;
}

DecisionResult IntegrationHub::run_cycle(DecisionEngine& engine, std::uint64_t now_ms) {
    health_monitor_.on_cycle(now_ms);
    poll_inputs(now_ms);

    last_quality_ = quality_monitor_.evaluate(last_p1_, last_p2_, last_env_, last_op_, now_ms);
    const auto raw_input = adapter_.to_sensor_input(now_ms);
    const auto imputed = SensorImputer::apply(raw_input, last_quality_);

    if (quality_monitor_.should_force_return_home(last_quality_, imputed.input.battery_level)
        || !health_monitor_.is_healthy()) {
        DecisionResult forced{};
        forced.selected_mode = OperationalMode::ReturnHome;
        forced.selected_score = 1.0f;
        forced.confidence = 0.99f;
        forced.emergency_forced = true;
        forced.degraded_inputs = last_quality_.degraded_inputs;
        forced.mode_changed = forced.selected_mode != engine.current_mode();
        forced.timestamp_ms = now_ms;
        forced.control_parameters = ModeParameterMapper::for_mode(
            OperationalMode::ReturnHome, imputed.input, forced.selected_score);
        publisher_.publish(forced);
        return forced;
    }

    EvaluateContext context{};
    context.quality = &last_quality_;
    context.imputation = &imputed.weights;
    const auto result = engine.evaluate(imputed.input, now_ms, context);
    publisher_.publish(result);
    return result;
}

bool IntegrationHub::product1_connected() const {
    return product1_ != nullptr && product1_->is_connected();
}

bool IntegrationHub::product2_connected() const {
    return product2_ != nullptr && product2_->is_connected();
}

const DataQualityStatus& IntegrationHub::last_quality_status() const {
    return last_quality_;
}

const SystemHealthMonitor& IntegrationHub::health_monitor() const {
    return health_monitor_;
}

bool IntegrationHub::is_healthy() const {
    return health_monitor_.is_healthy();
}

void CanProduct1Client::on_frame(const protocol::Product1CanPayload& payload, std::uint64_t timestamp_ms) {
    latest_ = protocol::decode_product1(payload, timestamp_ms);
}

bool CanProduct1Client::poll(Product1Data& out, std::uint64_t now_ms) {
    if (!latest_.valid) {
        return false;
    }
    if (now_ms > latest_.timestamp_ms + kDefaultStaleThresholdMs) {
        return false;
    }
    out = latest_;
    return true;
}

bool CanProduct1Client::is_connected() const {
    return latest_.valid;
}

void CanProduct2Client::on_frame(const protocol::Product2CanPayload& payload, std::uint64_t timestamp_ms) {
    latest_ = protocol::decode_product2(payload, timestamp_ms);
}

bool CanProduct2Client::poll(Product2Data& out, std::uint64_t now_ms) {
    if (!latest_.valid) {
        return false;
    }
    if (now_ms > latest_.timestamp_ms + kDefaultStaleThresholdMs) {
        return false;
    }
    out = latest_;
    return true;
}

bool CanProduct2Client::is_connected() const {
    return latest_.valid;
}

CanControlPublisher::CanControlPublisher(SendFrameFn send_frame)
    : send_frame_(std::move(send_frame)) {}

bool CanControlPublisher::publish(const DecisionResult& result) {
    last_ = protocol::encode_mode_command(result);
    if (!send_frame_) {
        return false;
    }
    return send_frame_(
        protocol::kCanIdModeCommand,
        &last_,
        sizeof(last_));
}

const protocol::ModeCommandPayload& CanControlPublisher::last_payload() const {
    return last_;
}

}  // namespace mili

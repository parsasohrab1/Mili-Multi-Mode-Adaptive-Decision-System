#include "mili/protocol/messages.hpp"

namespace mili::protocol {

namespace {

std::uint8_t encode_factor(float value) {
    if (value < 0.0f) {
        return 0;
    }
    if (value > 1.0f) {
        return 100;
    }
    return static_cast<std::uint8_t>(value * 100.0f);
}

float decode_factor(std::uint8_t encoded) {
    return static_cast<float>(encoded) / 100.0f;
}

std::uint8_t encode_radius(float km) {
    if (km < 0.0f) {
        return 0;
    }
    if (km > 25.5f) {
        return 255;
    }
    return static_cast<std::uint8_t>(km * 10.0f);
}

float decode_radius(std::uint8_t encoded) {
    return static_cast<float>(encoded) / 10.0f;
}

Product1Data decode_product1_base(const Product1CanPayload& payload) {
    Product1Data data{};
    data.battery_level_percent = static_cast<float>(payload.battery_level_x100) / 100.0f;
    data.power_draw_watts = static_cast<float>(payload.power_draw_x10) / 10.0f;
    data.remaining_flight_time_min = static_cast<float>(payload.remaining_time_min);
    data.power_budget_watts = static_cast<float>(payload.power_budget_x10) / 10.0f;
    data.low_power_warning = (payload.flags & 0x01) != 0;
    data.sequence = payload.sequence;
    data.schema_version = static_cast<std::uint8_t>((payload.flags >> 4) & 0x0F);
    if (data.schema_version == 0) {
        data.schema_version = static_cast<std::uint8_t>(schema::kContractVersion);
    }
    data.valid = true;
    return data;
}

}  // namespace

Product1Data decode_product1(const Product1CanPayload& payload, std::uint64_t timestamp_ms) {
    auto data = decode_product1_base(payload);
    data.timestamp_ms = timestamp_ms;
    data.source_timestamp_ms = timestamp_ms;
    return data;
}

Product1Data decode_product1(
    const Product1CanPayload& payload,
    const schema::ContractHeader& header,
    std::uint64_t rx_timestamp_ms) {
    auto data = decode_product1_base(payload);
    if (!schema::validate_header(header, schema::DataSourceId::Product1Power)) {
        data.valid = false;
        return data;
    }
    data.schema_version = header.schema_version;
    data.sequence = header.sequence;
    data.source_timestamp_ms = header.source_timestamp_ms;
    data.timestamp_ms = rx_timestamp_ms;
    return data;
}

Product2Data decode_product2(const Product2CanPayload& payload, std::uint64_t timestamp_ms) {
    Product2Data data{};
    data.position_x_km = static_cast<float>(payload.position_x_dm) / 10.0f;
    data.position_y_km = static_cast<float>(payload.position_y_dm) / 10.0f;
    data.distance_to_home_km = static_cast<float>(payload.distance_home_x10) / 10.0f;
    data.velocity_mps = static_cast<float>(payload.velocity_x10) / 10.0f;
    data.schema_version = static_cast<std::uint8_t>(schema::kContractVersion);
    data.valid = true;
    data.timestamp_ms = timestamp_ms;
    data.source_timestamp_ms = timestamp_ms;
    return data;
}

void apply_vio_extension(Product2Data& data, const Product2VioExtPayload& vio_payload) {
    data.heading_deg = static_cast<float>(vio_payload.heading_x10) / 10.0f;
    data.altitude_m = static_cast<float>(vio_payload.altitude_m);
    data.vio_confidence = static_cast<float>(vio_payload.vio_confidence_x100) / 100.0f;
    data.schema_version = vio_payload.schema_version;
    data.sequence = vio_payload.sequence;
}

Product2Data decode_product2(
    const Product2CanPayload& nav_payload,
    const Product2VioExtPayload& vio_payload,
    const schema::ContractHeader& header,
    std::uint64_t rx_timestamp_ms) {
    auto data = decode_product2(nav_payload, rx_timestamp_ms);
    if (!schema::validate_header(header, schema::DataSourceId::Product2Vio)) {
        data.valid = false;
        return data;
    }
    apply_vio_extension(data, vio_payload);
    data.schema_version = header.schema_version;
    data.sequence = header.sequence;
    data.source_timestamp_ms = header.source_timestamp_ms;
    return data;
}

OperatorCommand decode_operator(const OperatorCanPayload& payload, std::uint64_t timestamp_ms) {
    OperatorCommand command{};
    command.valid = true;
    command.has_priority_override = (payload.flags & 0x01) != 0;
    command.priority_override = static_cast<MissionPriority>(payload.priority_override);
    command.timestamp_ms = timestamp_ms;
    return command;
}

ModeCommandPayload encode_mode_command(const DecisionResult& result) {
    ModeCommandPayload payload{};
    payload.mode = static_cast<std::uint8_t>(result.selected_mode);
    payload.speed_factor_x100 = encode_factor(result.control_parameters.speed_factor);
    payload.altitude_factor_x100 = encode_factor(result.control_parameters.altitude_factor);
    payload.sensor_aggressiveness_x100 = encode_factor(result.control_parameters.sensor_aggressiveness);
    payload.engagement_readiness_x100 = encode_factor(result.control_parameters.engagement_readiness);
    payload.protocol_version = static_cast<std::uint8_t>(kProtocolVersion);
    return payload;
}

ControlParametersPayload encode_control_parameters(const DecisionResult& result) {
    ControlParametersPayload payload{};
    payload.mode = static_cast<std::uint8_t>(result.selected_mode);
    payload.loiter_radius_x10 = encode_radius(result.control_parameters.loiter_radius_km);
    payload.return_urgency_x100 = encode_factor(result.control_parameters.return_urgency);
    payload.protocol_version = static_cast<std::uint8_t>(kProtocolVersion);
    payload.speed_factor_x100 = encode_factor(result.control_parameters.speed_factor);
    payload.altitude_factor_x100 = encode_factor(result.control_parameters.altitude_factor);
    payload.sensor_aggressiveness_x100 = encode_factor(result.control_parameters.sensor_aggressiveness);
    payload.engagement_readiness_x100 = encode_factor(result.control_parameters.engagement_readiness);
    return payload;
}

ModeParameters decode_control_parameters(const ControlParametersPayload& payload) {
    ModeParameters params{};
    params.speed_factor = decode_factor(payload.speed_factor_x100);
    params.altitude_factor = decode_factor(payload.altitude_factor_x100);
    params.sensor_aggressiveness = decode_factor(payload.sensor_aggressiveness_x100);
    params.engagement_readiness = decode_factor(payload.engagement_readiness_x100);
    params.loiter_radius_km = decode_radius(payload.loiter_radius_x10);
    params.return_urgency = decode_factor(payload.return_urgency_x100);
    return params;
}

DecisionReportPayload encode_decision_report(const DecisionResult& result, std::uint16_t evaluate_time_us) {
    DecisionReportPayload payload{};
    payload.mode = static_cast<std::uint8_t>(result.selected_mode);
    payload.confidence_x100 = encode_factor(result.confidence);
    const float clamped_score = result.selected_score < 0.0f ? 0.0f
        : (result.selected_score > 1.0f ? 1.0f : result.selected_score);
    payload.score_x1000 = static_cast<std::uint16_t>(clamped_score * 1000.0f);
    payload.mode_changed = result.mode_changed ? 1 : 0;
    payload.protocol_version = static_cast<std::uint8_t>(kProtocolVersion);
    payload.evaluate_time_us = evaluate_time_us;
    return payload;
}

DecisionResult decode_mode_command(const ModeCommandPayload& payload) {
    DecisionResult result{};
    result.selected_mode = static_cast<OperationalMode>(payload.mode);
    result.control_parameters.speed_factor = decode_factor(payload.speed_factor_x100);
    result.control_parameters.altitude_factor = decode_factor(payload.altitude_factor_x100);
    result.control_parameters.sensor_aggressiveness = decode_factor(payload.sensor_aggressiveness_x100);
    result.control_parameters.engagement_readiness = decode_factor(payload.engagement_readiness_x100);
    return result;
}

ConfigUpdatePayload encode_config_update(
    ConfigCommand command, OperationalMode mode, std::uint8_t factor_id, float value) {
    ConfigUpdatePayload payload{};
    payload.command = static_cast<std::uint8_t>(command);
    payload.mode = static_cast<std::uint8_t>(mode);
    payload.factor_id = factor_id;
    payload.value_x100 = encode_factor(value);
    payload.checksum = static_cast<std::uint32_t>(payload.command)
                     ^ static_cast<std::uint32_t>(payload.mode << 8)
                     ^ static_cast<std::uint32_t>(payload.factor_id << 16)
                     ^ static_cast<std::uint32_t>(payload.value_x100 << 24);
    return payload;
}

bool decode_config_update(
    const ConfigUpdatePayload& payload,
    ConfigCommand& command,
    OperationalMode& mode,
    std::uint8_t& factor_id,
    float& value) {
    const std::uint32_t expected = static_cast<std::uint32_t>(payload.command)
                                 ^ static_cast<std::uint32_t>(payload.mode << 8)
                                 ^ static_cast<std::uint32_t>(payload.factor_id << 16)
                                 ^ static_cast<std::uint32_t>(payload.value_x100 << 24);
    if (payload.checksum != expected) {
        return false;
    }
    command = static_cast<ConfigCommand>(payload.command);
    mode = static_cast<OperationalMode>(payload.mode);
    factor_id = payload.factor_id;
    value = decode_factor(payload.value_x100);
    return true;
}

}  // namespace mili::protocol

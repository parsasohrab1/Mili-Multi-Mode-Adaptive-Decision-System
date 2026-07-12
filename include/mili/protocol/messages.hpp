#pragma once

#include "mili/product_data.hpp"
#include "mili/schema/data_contract.hpp"
#include "mili/types.hpp"

#include <cstdint>

namespace mili::protocol {

constexpr std::uint32_t kProtocolVersion = 1;

constexpr std::uint16_t kCanIdProduct1Status = 0x301;
constexpr std::uint16_t kCanIdProduct2Nav = 0x302;
constexpr std::uint16_t kCanIdOperatorCmd = 0x303;
constexpr std::uint16_t kCanIdProduct2VioExt = 0x304;
constexpr std::uint16_t kCanIdModeCommand = 0x310;
constexpr std::uint16_t kCanIdDecisionReport = 0x311;
constexpr std::uint16_t kCanIdConfigUpdate = 0x312;
constexpr std::uint16_t kCanIdControlParams = 0x313;

enum class ConfigCommand : std::uint8_t {
    ReloadDefaults = 0,
    SetHysteresis = 1,
    SetModeFactor = 2,
};

#pragma pack(push, 1)
struct Product1CanPayload {
    std::uint16_t battery_level_x100;
    std::uint16_t power_draw_x10;
    std::uint8_t flags;
    std::uint8_t sequence;
    std::uint8_t remaining_time_min;
    std::uint8_t power_budget_x10;
};

struct Product2CanPayload {
    std::int16_t position_x_dm;
    std::int16_t position_y_dm;
    std::uint16_t distance_home_x10;
    std::uint16_t velocity_x10;
};

struct Product2VioExtPayload {
    std::uint16_t heading_x10;
    std::uint16_t altitude_m;
    std::uint8_t vio_confidence_x100;
    std::uint8_t schema_version;
    std::uint8_t sequence;
    std::uint8_t reserved;
};

struct ControlParametersPayload {
    std::uint8_t mode;
    std::uint8_t loiter_radius_x10;
    std::uint8_t return_urgency_x100;
    std::uint8_t protocol_version;
    std::uint8_t speed_factor_x100;
    std::uint8_t altitude_factor_x100;
    std::uint8_t sensor_aggressiveness_x100;
    std::uint8_t engagement_readiness_x100;
};

struct OperatorCanPayload {
    std::uint8_t priority_override;
    std::uint8_t flags;
    std::uint16_t reserved;
};

struct ModeCommandPayload {
    std::uint8_t mode;
    std::uint8_t speed_factor_x100;
    std::uint8_t altitude_factor_x100;
    std::uint8_t sensor_aggressiveness_x100;
    std::uint8_t engagement_readiness_x100;
    std::uint8_t protocol_version;
    std::uint8_t reserved[2];
};

struct DecisionReportPayload {
    std::uint8_t mode;
    std::uint8_t confidence_x100;
    std::uint16_t score_x1000;
    std::uint8_t mode_changed;
    std::uint8_t protocol_version;
    std::uint16_t evaluate_time_us;
};

struct ConfigUpdatePayload {
    std::uint8_t command;
    std::uint8_t mode;
    std::uint8_t factor_id;
    std::uint8_t value_x100;
    std::uint32_t checksum;
};
#pragma pack(pop)

Product1Data decode_product1(const Product1CanPayload& payload, std::uint64_t timestamp_ms);
Product1Data decode_product1(
    const Product1CanPayload& payload,
    const schema::ContractHeader& header,
    std::uint64_t rx_timestamp_ms);
Product2Data decode_product2(const Product2CanPayload& payload, std::uint64_t timestamp_ms);
Product2Data decode_product2(
    const Product2CanPayload& nav_payload,
    const Product2VioExtPayload& vio_payload,
    const schema::ContractHeader& header,
    std::uint64_t rx_timestamp_ms);
void apply_vio_extension(Product2Data& data, const Product2VioExtPayload& vio_payload);
OperatorCommand decode_operator(const OperatorCanPayload& payload, std::uint64_t timestamp_ms);

ModeCommandPayload encode_mode_command(const DecisionResult& result);
ControlParametersPayload encode_control_parameters(const DecisionResult& result);
DecisionReportPayload encode_decision_report(const DecisionResult& result, std::uint16_t evaluate_time_us);
DecisionResult decode_mode_command(const ModeCommandPayload& payload);
ModeParameters decode_control_parameters(const ControlParametersPayload& payload);

ConfigUpdatePayload encode_config_update(ConfigCommand command, OperationalMode mode, std::uint8_t factor_id, float value);
bool decode_config_update(const ConfigUpdatePayload& payload, ConfigCommand& command, OperationalMode& mode, std::uint8_t& factor_id, float& value);

}  // namespace mili::protocol

#pragma once

#include <cstdint>

namespace mili::schema {

constexpr std::uint32_t kContractVersion = 1;

enum class DataSourceId : std::uint8_t {
    Product1Power = 1,
    Product2Vio = 2,
    Environmental = 3,
    Operator = 4,
    DecisionOutput = 5,
};

#pragma pack(push, 1)
struct ContractHeader {
    std::uint8_t schema_version;
    std::uint8_t source_id;
    std::uint16_t sequence;
    std::uint32_t source_timestamp_ms;
};
#pragma pack(pop)

std::uint32_t header_checksum(const ContractHeader& header);
bool validate_header(const ContractHeader& header, DataSourceId expected_source);
bool is_compatible_version(std::uint8_t schema_version);

}  // namespace mili::schema

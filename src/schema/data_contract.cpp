#include "mili/schema/data_contract.hpp"

namespace mili::schema {

std::uint32_t header_checksum(const ContractHeader& header) {
    return static_cast<std::uint32_t>(header.schema_version)
         ^ static_cast<std::uint32_t>(header.source_id << 8)
         ^ static_cast<std::uint32_t>(header.sequence << 16)
         ^ header.source_timestamp_ms;
}

bool is_compatible_version(std::uint8_t schema_version) {
    return schema_version == static_cast<std::uint8_t>(kContractVersion);
}

bool validate_header(const ContractHeader& header, DataSourceId expected_source) {
    if (!is_compatible_version(header.schema_version)) {
        return false;
    }
    if (header.source_id != static_cast<std::uint8_t>(expected_source)) {
        return false;
    }
    if (header.source_timestamp_ms == 0) {
        return false;
    }
    return true;
}

}  // namespace mili::schema

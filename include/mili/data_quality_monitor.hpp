#pragma once

#include "mili/data_quality_monitor.hpp"
#include "mili/product_data.hpp"
#include "mili/types.hpp"

#include <cstdint>

namespace mili {

enum class SourceQuality : std::uint8_t {
    Ok = 0,
    Missing,
    Stale,
    Invalid,
};

struct DataQualityStatus {
    SourceQuality product1_quality = SourceQuality::Missing;
    SourceQuality product2_quality = SourceQuality::Missing;
    SourceQuality environmental_quality = SourceQuality::Missing;
    SourceQuality operator_quality = SourceQuality::Missing;

    bool product1_valid = false;
    bool product2_valid = false;
    bool environmental_valid = false;
    bool operator_valid = false;
    bool product1_stale = false;
    bool product2_stale = false;
    bool environmental_stale = false;
    bool product1_invalid = false;
    bool product2_invalid = false;
    bool environmental_invalid = false;

    std::uint8_t missing_count = 0;
    std::uint8_t stale_count = 0;
    std::uint8_t invalid_count = 0;
    std::uint8_t degraded_count = 0;

    bool emergency_return = false;
    bool degraded_inputs = false;
    float imputation_factor = 1.0f;
};

class DataQualityMonitor {
public:
    explicit DataQualityMonitor(std::uint64_t stale_threshold_ms = kDefaultStaleThresholdMs);

    DataQualityStatus evaluate(
        const Product1Data& p1,
        const Product2Data& p2,
        const EnvironmentalData& env,
        const OperatorCommand& op,
        std::uint64_t now_ms) const;

    bool should_force_return_home(const DataQualityStatus& status, float battery_level) const;
    std::uint8_t min_missing_for_emergency() const;

    static bool is_valid_product1(const Product1Data& data);
    static bool is_valid_product2(const Product2Data& data);
    static bool is_valid_environmental(const EnvironmentalData& data);

private:
    std::uint64_t stale_threshold_ms_;
    std::uint8_t min_missing_for_emergency_;

    static bool is_stale(std::uint64_t timestamp_ms, std::uint64_t now_ms, std::uint64_t threshold_ms);
    static SourceQuality classify(bool present, bool stale, bool invalid);
};

}  // namespace mili

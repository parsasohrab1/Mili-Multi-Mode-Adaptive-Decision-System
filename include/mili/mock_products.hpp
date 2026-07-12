#pragma once

#include "mili/product_data.hpp"

namespace mili {

class MockProduct1 {
public:
    Product1Data sample(std::uint64_t timestamp_ms, float battery_level = 75.0f) const;
    Product1Data invalid_sample() const;
};

class MockProduct2 {
public:
    Product2Data sample(std::uint64_t timestamp_ms, float distance_km = 6.0f) const;
    Product2Data invalid_sample() const;
};

EnvironmentalData mock_environmental(std::uint64_t timestamp_ms);

}  // namespace mili

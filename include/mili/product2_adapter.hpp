#pragma once

#include "mili/product_data.hpp"

namespace mili {

class Product2Adapter {
public:
    void update(const Product2Data& data, std::uint64_t now_ms);

    const Product2Data& latest_valid() const;
    bool has_valid_data() const;
    bool is_stale(std::uint64_t now_ms, std::uint64_t threshold_ms = kDefaultStaleThresholdMs) const;

private:
    Product2Data latest_{};
};

}  // namespace mili

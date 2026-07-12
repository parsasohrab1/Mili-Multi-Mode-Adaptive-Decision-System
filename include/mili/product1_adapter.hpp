#pragma once

#include "mili/product_data.hpp"

namespace mili {

class Product1Adapter {
public:
    void update(const Product1Data& data, std::uint64_t now_ms);

    const Product1Data& latest_valid() const;
    bool has_valid_data() const;
    bool is_stale(std::uint64_t now_ms, std::uint64_t threshold_ms = kDefaultStaleThresholdMs) const;

private:
    Product1Data latest_{};
};

}  // namespace mili

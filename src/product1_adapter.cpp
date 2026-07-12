#include "mili/product1_adapter.hpp"

namespace mili {

void Product1Adapter::update(const Product1Data& data, std::uint64_t now_ms) {
    if (!data.valid) {
        return;
    }

    latest_ = data;
    latest_.timestamp_ms = now_ms;
}

const Product1Data& Product1Adapter::latest_valid() const {
    return latest_;
}

bool Product1Adapter::has_valid_data() const {
    return latest_.valid;
}

bool Product1Adapter::is_stale(std::uint64_t now_ms, std::uint64_t threshold_ms) const {
    if (!latest_.valid) {
        return true;
    }
    return now_ms > latest_.timestamp_ms + threshold_ms;
}

}  // namespace mili

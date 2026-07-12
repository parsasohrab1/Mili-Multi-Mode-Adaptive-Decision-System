#include "mili/product2_adapter.hpp"

namespace mili {

void Product2Adapter::update(const Product2Data& data, std::uint64_t now_ms) {
    if (!data.valid) {
        return;
    }

    latest_ = data;
    latest_.timestamp_ms = now_ms;
}

const Product2Data& Product2Adapter::latest_valid() const {
    return latest_;
}

bool Product2Adapter::has_valid_data() const {
    return latest_.valid;
}

bool Product2Adapter::is_stale(std::uint64_t now_ms, std::uint64_t threshold_ms) const {
    if (!latest_.valid) {
        return true;
    }
    return now_ms > latest_.timestamp_ms + threshold_ms;
}

}  // namespace mili

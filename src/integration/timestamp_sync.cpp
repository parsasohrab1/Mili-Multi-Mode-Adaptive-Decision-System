#include "mili/integration/timestamp_sync.hpp"

#include <cmath>

namespace mili {

std::size_t TimestampSynchronizer::index_for(schema::DataSourceId source) {
    const auto raw = static_cast<std::uint8_t>(source);
    if (raw == 0 || raw > 5) {
        return 0;
    }
    return static_cast<std::size_t>(raw);
}

void TimestampSynchronizer::observe(
    schema::DataSourceId source, std::uint64_t source_ts_ms, std::uint64_t local_rx_ms) {
    auto& clock = clocks_[index_for(source)];
    if (clock.observed && local_rx_ms > clock.last_local_ms) {
        const float dt_s = static_cast<float>(local_rx_ms - clock.last_local_ms) / 1000.0f;
        const auto source_delta = static_cast<std::int64_t>(source_ts_ms) - static_cast<std::int64_t>(clock.last_source_ms);
        const auto local_delta = static_cast<std::int64_t>(local_rx_ms) - static_cast<std::int64_t>(clock.last_local_ms);
        if (dt_s > 0.01f) {
            const float drift = static_cast<float>(source_delta - local_delta) / dt_s;
            clock.drift_ms_per_s = 0.8f * clock.drift_ms_per_s + 0.2f * drift;
        }
    }

    clock.offset_ms = static_cast<std::int64_t>(local_rx_ms) - static_cast<std::int64_t>(source_ts_ms);
    clock.last_local_ms = local_rx_ms;
    clock.last_source_ms = source_ts_ms;
    clock.observed = true;
}

std::uint64_t TimestampSynchronizer::aligned_timestamp(
    schema::DataSourceId source, std::uint64_t source_ts_ms) const {
    const auto& clock = clocks_[index_for(source)];
    if (!clock.observed) {
        return source_ts_ms;
    }
    const auto aligned = static_cast<std::int64_t>(source_ts_ms) + clock.offset_ms;
    return aligned < 0 ? 0 : static_cast<std::uint64_t>(aligned);
}

float TimestampSynchronizer::drift_ms(schema::DataSourceId source) const {
    return clocks_[index_for(source)].drift_ms_per_s;
}

bool TimestampSynchronizer::is_synchronized(std::uint64_t now_ms, std::uint64_t max_skew_ms) const {
    bool p1 = false;
    bool p2 = false;
    for (std::size_t i = 0; i < 6; ++i) {
        const auto& clock = clocks_[i];
        if (!clock.observed) {
            continue;
        }
        const auto skew = now_ms > clock.last_local_ms ? now_ms - clock.last_local_ms : 0;
        if (skew > max_skew_ms) {
            return false;
        }
        if (i == index_for(schema::DataSourceId::Product1Power)) {
            p1 = true;
        }
        if (i == index_for(schema::DataSourceId::Product2Vio)) {
            p2 = true;
        }
    }
    return p1 && p2;
}

const SourceClockState& TimestampSynchronizer::state(schema::DataSourceId source) const {
    return clocks_[index_for(source)];
}

}  // namespace mili

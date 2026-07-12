#pragma once

#include "mili/schema/data_contract.hpp"

#include <cstddef>
#include <cstdint>

namespace mili {

struct SourceClockState {
    std::int64_t offset_ms = 0;
    float drift_ms_per_s = 0.0f;
    std::uint64_t last_local_ms = 0;
    std::uint64_t last_source_ms = 0;
    bool observed = false;
};

class TimestampSynchronizer {
public:
    void observe(schema::DataSourceId source, std::uint64_t source_ts_ms, std::uint64_t local_rx_ms);
    std::uint64_t aligned_timestamp(schema::DataSourceId source, std::uint64_t source_ts_ms) const;
    float drift_ms(schema::DataSourceId source) const;
    bool is_synchronized(std::uint64_t now_ms, std::uint64_t max_skew_ms = 100) const;
    const SourceClockState& state(schema::DataSourceId source) const;

private:
    SourceClockState clocks_[6]{};
    static std::size_t index_for(schema::DataSourceId source);
};

}  // namespace mili

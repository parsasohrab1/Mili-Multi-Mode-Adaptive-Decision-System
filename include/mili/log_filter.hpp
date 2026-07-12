#pragma once

#include "mili/decision_logger_interface.hpp"
#include "mili/types.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mili {

struct LogQuery {
    OperationalMode mode_filter = OperationalMode::Count;
    float min_confidence = 0.0f;
    float max_confidence = 1.0f;
    std::uint64_t start_timestamp_ms = 0;
    std::uint64_t end_timestamp_ms = UINT64_MAX;
    bool low_confidence_only = false;
    bool degraded_only = false;
    bool emergency_only = false;
    bool mode_changed_only = false;
};

class LogFilter {
public:
    static std::vector<DecisionLogEntry> apply(const IDecisionLogger& logger, const LogQuery& query);
    static std::size_t count(const IDecisionLogger& logger, const LogQuery& query);
    static bool matches(const DecisionLogEntry& entry, const LogQuery& query);
};

}  // namespace mili

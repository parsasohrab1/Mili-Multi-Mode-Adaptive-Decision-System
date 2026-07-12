#include "mili/log_filter.hpp"

namespace mili {

bool LogFilter::matches(const DecisionLogEntry& entry, const LogQuery& query) {
    if (entry.timestamp_ms < query.start_timestamp_ms || entry.timestamp_ms > query.end_timestamp_ms) {
        return false;
    }
    if (query.mode_filter != OperationalMode::Count && entry.result.selected_mode != query.mode_filter) {
        return false;
    }
    if (entry.result.confidence < query.min_confidence || entry.result.confidence > query.max_confidence) {
        return false;
    }
    if (query.low_confidence_only && !entry.result.low_confidence) {
        return false;
    }
    if (query.degraded_only && !entry.result.degraded_inputs) {
        return false;
    }
    if (query.emergency_only && !entry.result.emergency_forced) {
        return false;
    }
    if (query.mode_changed_only && !entry.result.mode_changed) {
        return false;
    }
    return true;
}

std::vector<DecisionLogEntry> LogFilter::apply(const IDecisionLogger& logger, const LogQuery& query) {
    std::vector<DecisionLogEntry> matches;
    DecisionLogEntry entry{};
    for (std::size_t i = 0; i < logger.size(); ++i) {
        if (!logger.entry_at(i, entry)) {
            continue;
        }
        if (LogFilter::matches(entry, query)) {
            matches.push_back(entry);
        }
    }
    return matches;
}

std::size_t LogFilter::count(const IDecisionLogger& logger, const LogQuery& query) {
    std::size_t total = 0;
    DecisionLogEntry entry{};
    for (std::size_t i = 0; i < logger.size(); ++i) {
        if (!logger.entry_at(i, entry)) {
            continue;
        }
        if (LogFilter::matches(entry, query)) {
            ++total;
        }
    }
    return total;
}

}  // namespace mili

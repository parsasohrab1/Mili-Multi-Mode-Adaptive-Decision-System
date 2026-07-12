#include "mili/decision_logger.hpp"

namespace mili {

DecisionLogger::DecisionLogger(std::size_t max_entries)
    : max_entries_(max_entries) {}

void DecisionLogger::log(
    const SensorInput& input, const DecisionResult& result, std::uint64_t timestamp_ms) {
    entries_.push_back(DecisionLogEntry{timestamp_ms, input, result});
    if (entries_.size() > max_entries_) {
        entries_.erase(entries_.begin());
    }
}

std::size_t DecisionLogger::size() const {
    return entries_.size();
}

std::size_t DecisionLogger::capacity() const {
    return max_entries_;
}

void DecisionLogger::clear() {
    entries_.clear();
}

bool DecisionLogger::entry_at(std::size_t index, DecisionLogEntry& out) const {
    if (index >= entries_.size()) {
        return false;
    }
    out = entries_[index];
    return true;
}

const std::vector<DecisionLogEntry>& DecisionLogger::entries() const {
    return entries_;
}

}  // namespace mili

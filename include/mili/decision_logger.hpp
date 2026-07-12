#pragma once

#include "mili/decision_logger_interface.hpp"
#include "mili/types.hpp"

#include <cstddef>
#include <vector>

namespace mili {

class DecisionLogger : public IDecisionLogger {
public:
    explicit DecisionLogger(std::size_t max_entries = kMaxLogEntries);

    void log(
        const SensorInput& input,
        const DecisionResult& result,
        std::uint64_t timestamp_ms) override;

    std::size_t size() const override;
    std::size_t capacity() const override;
    void clear() override;
    bool entry_at(std::size_t index, DecisionLogEntry& out) const override;

    const std::vector<DecisionLogEntry>& entries() const;

private:
    std::size_t max_entries_;
    std::vector<DecisionLogEntry> entries_;
};

}  // namespace mili

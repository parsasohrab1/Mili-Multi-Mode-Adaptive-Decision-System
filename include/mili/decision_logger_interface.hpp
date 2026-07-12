#pragma once

#include "mili/types.hpp"

#include <cstddef>

namespace mili {

class IDecisionLogger {
public:
    virtual ~IDecisionLogger() = default;

    virtual void log(
        const SensorInput& input,
        const DecisionResult& result,
        std::uint64_t timestamp_ms) = 0;

    virtual std::size_t size() const = 0;
    virtual std::size_t capacity() const = 0;
    virtual void clear() = 0;

    virtual bool entry_at(std::size_t index, DecisionLogEntry& out) const = 0;
};

}  // namespace mili

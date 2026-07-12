#pragma once

#include "mili/decision_logger_interface.hpp"
#include "mili/embedded_config.hpp"
#include "mili/types.hpp"

#include <array>
#include <cstddef>

namespace mili {

template <std::size_t Capacity>
class RingBufferLogger : public IDecisionLogger {
public:
    void log(
        const SensorInput& input,
        const DecisionResult& result,
        std::uint64_t timestamp_ms) override;

    std::size_t size() const override;
    std::size_t capacity() const override;
    void clear() override;
    bool entry_at(std::size_t index, DecisionLogEntry& out) const override;

    const DecisionLogEntry& at(std::size_t index) const;

private:
    std::array<DecisionLogEntry, Capacity> buffer_{};
    std::size_t head_ = 0;
    std::size_t count_ = 0;
};

using EmbeddedDecisionLogger = RingBufferLogger<kEmbeddedLogCapacity>;

}  // namespace mili

#include "mili/ring_buffer_logger.inl"

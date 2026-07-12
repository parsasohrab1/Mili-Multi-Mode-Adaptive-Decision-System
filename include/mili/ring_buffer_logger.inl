#pragma once

#include "mili/types.hpp"

namespace mili {

template <std::size_t Capacity>
void RingBufferLogger<Capacity>::log(
    const SensorInput& input, const DecisionResult& result, std::uint64_t timestamp_ms) {
    buffer_[head_] = DecisionLogEntry{timestamp_ms, input, result};
    head_ = (head_ + 1) % Capacity;
    if (count_ < Capacity) {
        ++count_;
    }
}

template <std::size_t Capacity>
std::size_t RingBufferLogger<Capacity>::size() const {
    return count_;
}

template <std::size_t Capacity>
std::size_t RingBufferLogger<Capacity>::capacity() const {
    return Capacity;
}

template <std::size_t Capacity>
void RingBufferLogger<Capacity>::clear() {
    head_ = 0;
    count_ = 0;
}

template <std::size_t Capacity>
bool RingBufferLogger<Capacity>::entry_at(std::size_t index, DecisionLogEntry& out) const {
    if (index >= count_) {
        return false;
    }
    out = at(index);
    return true;
}

template <std::size_t Capacity>
const DecisionLogEntry& RingBufferLogger<Capacity>::at(std::size_t index) const {
    const std::size_t start = (head_ + Capacity - count_) % Capacity;
    return buffer_[(start + index) % Capacity];
}

}  // namespace mili

#pragma once

#include "mili/types.hpp"

#include <cstddef>

namespace mili {

constexpr std::size_t kEmbeddedLogCapacity = 32;
constexpr std::size_t kEmbeddedRamBudgetBytes = 64 * 1024;

#ifdef MILI_EMBEDDED
constexpr std::size_t kPlatformLogCapacity = kEmbeddedLogCapacity;
constexpr float kEmbeddedEvaluateBudgetMs = 5.0f;
constexpr float kEmbeddedCycleBudgetMs = 50.0f;
#else
constexpr std::size_t kPlatformLogCapacity = kMaxLogEntries;
#endif

constexpr std::size_t kEstimatedLogEntryBytes = 160;

inline constexpr std::size_t estimated_log_ram_bytes(std::size_t capacity) {
    return capacity * kEstimatedLogEntryBytes;
}

}  // namespace mili

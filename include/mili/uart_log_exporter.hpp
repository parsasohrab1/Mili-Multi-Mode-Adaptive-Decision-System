#pragma once

#include "mili/decision_logger_interface.hpp"

#include <cstddef>
#include <cstdint>

namespace mili {

using UartWriteCallback = bool (*)(const char* data, std::size_t len, void* user_data);

// Streams decision log entries over UART in CSV lines (embedded-safe, no heap).
class UartLogExporter {
public:
    explicit UartLogExporter(UartWriteCallback write, void* user_data = nullptr);

    bool write_header();
    std::size_t drain(const IDecisionLogger& logger, std::size_t max_lines = 0);
    std::size_t drain_range(
        const IDecisionLogger& logger,
        std::size_t start_index,
        std::size_t max_lines);

    std::size_t lines_sent() const;
    void reset_counter();

private:
    UartWriteCallback write_;
    void* user_data_;
    std::size_t lines_sent_;

    bool write_line(const char* line, std::size_t len);
};

}  // namespace mili

#include "mili/uart_log_exporter.hpp"

#include "mili/log_format.hpp"

#include <cstring>

namespace mili {

UartLogExporter::UartLogExporter(UartWriteCallback write, void* user_data)
    : write_(write)
    , user_data_(user_data)
    , lines_sent_(0) {}

bool UartLogExporter::write_line(const char* line, std::size_t len) {
    if (write_ == nullptr || line == nullptr || len == 0) {
        return false;
    }
    return write_(line, len, user_data_);
}

bool UartLogExporter::write_header() {
    const char* header = mili::logfmt::csv_header();
    return write_line(header, std::strlen(header));
}

std::size_t UartLogExporter::drain(const IDecisionLogger& logger, std::size_t max_lines) {
    const std::size_t limit = max_lines == 0 ? logger.size() : max_lines;
    return drain_range(logger, 0, limit);
}

std::size_t UartLogExporter::drain_range(
    const IDecisionLogger& logger, std::size_t start_index, std::size_t max_lines) {
    char line[mili::logfmt::kCsvLineMax];
    DecisionLogEntry entry{};
    std::size_t sent = 0;

    for (std::size_t i = start_index; i < logger.size() && sent < max_lines; ++i) {
        if (!logger.entry_at(i, entry)) {
            continue;
        }
        const auto len = mili::logfmt::format_csv_line(line, sizeof(line), entry);
        if (len == 0) {
            continue;
        }
        if (write_line(line, len)) {
            ++sent;
            ++lines_sent_;
        }
    }
    return sent;
}

std::size_t UartLogExporter::lines_sent() const {
    return lines_sent_;
}

void UartLogExporter::reset_counter() {
    lines_sent_ = 0;
}

}  // namespace mili

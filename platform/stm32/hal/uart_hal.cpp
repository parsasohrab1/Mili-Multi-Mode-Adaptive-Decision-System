#include "uart_hal.hpp"

#include <cstring>

namespace mili::platform {

UartHal::UartHal()
    : write_(nullptr)
    , write_ctx_(nullptr)
    , read_(nullptr)
    , read_ctx_(nullptr) {}

void UartHal::set_write_callback(UartWriteFn write, void* user_data) {
    write_ = write;
    write_ctx_ = user_data;
}

void UartHal::set_read_callback(UartReadFn read, void* user_data) {
    read_ = read;
    read_ctx_ = user_data;
}

void UartHal::write(const char* data, std::size_t len) {
    if (write_ != nullptr && data != nullptr && len > 0) {
        write_(data, len, write_ctx_);
    }
}

void UartHal::write_line(const char* line) {
    if (line == nullptr) {
        return;
    }
    write(line, std::strlen(line));
    write("\r\n", 2);
}

std::size_t UartHal::read_line(char* buffer, std::size_t capacity) {
    if (read_ == nullptr || buffer == nullptr || capacity == 0) {
        return 0;
    }
    return read_(buffer, capacity, read_ctx_);
}

void UartHal::drain_log_ring(
    const char* (*formatter)(std::uint64_t ts, char* buf, std::size_t cap), std::uint64_t timestamp_ms) {
    if (formatter == nullptr) {
        return;
    }
    char line[128];
    const char* formatted = formatter(timestamp_ms, line, sizeof(line));
    if (formatted != nullptr) {
        write_line(formatted);
    }
}

}  // namespace mili::platform

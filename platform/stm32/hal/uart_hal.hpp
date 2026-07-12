#pragma once

#include <cstddef>
#include <cstdint>

namespace mili::platform {

using UartWriteFn = void (*)(const char* data, std::size_t len, void* user_data);
using UartReadFn = std::size_t (*)(char* buffer, std::size_t capacity, void* user_data);

class UartHal {
public:
    UartHal();

    void set_write_callback(UartWriteFn write, void* user_data);
    void set_read_callback(UartReadFn read, void* user_data);

    void write(const char* data, std::size_t len);
    void write_line(const char* line);
    std::size_t read_line(char* buffer, std::size_t capacity);

    void drain_log_ring(const char* (*formatter)(std::uint64_t ts, char* buf, std::size_t cap), std::uint64_t timestamp_ms);

private:
    UartWriteFn write_;
    void* write_ctx_;
    UartReadFn read_;
    void* read_ctx_;
};

}  // namespace mili::platform

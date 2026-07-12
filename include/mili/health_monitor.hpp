#pragma once

#include <cstdint>

namespace mili {

class SystemHealthMonitor {
public:
    explicit SystemHealthMonitor(
        std::uint64_t expected_cycle_ms = 100,
        std::uint64_t watchdog_timeout_ms = 500);

    void on_cycle(std::uint64_t now_ms);
    void reset();

    bool is_healthy() const;
    bool watchdog_triggered() const;
    std::uint32_t missed_cycles() const;
    std::uint64_t last_cycle_ms() const;

private:
    std::uint64_t expected_cycle_ms_;
    std::uint64_t watchdog_timeout_ms_;
    std::uint64_t last_cycle_ms_;
    std::uint32_t missed_cycles_;
    bool watchdog_triggered_;
};

}  // namespace mili

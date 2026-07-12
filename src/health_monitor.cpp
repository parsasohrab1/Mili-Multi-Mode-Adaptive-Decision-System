#include "mili/health_monitor.hpp"

namespace mili {

SystemHealthMonitor::SystemHealthMonitor(std::uint64_t expected_cycle_ms, std::uint64_t watchdog_timeout_ms)
    : expected_cycle_ms_(expected_cycle_ms)
    , watchdog_timeout_ms_(watchdog_timeout_ms)
    , last_cycle_ms_(0)
    , missed_cycles_(0)
    , watchdog_triggered_(false) {}

void SystemHealthMonitor::on_cycle(std::uint64_t now_ms) {
    if (last_cycle_ms_ > 0 && now_ms > last_cycle_ms_) {
        const auto gap = now_ms - last_cycle_ms_;
        if (gap > watchdog_timeout_ms_) {
            watchdog_triggered_ = true;
        }
        if (gap > expected_cycle_ms_ * 2) {
            ++missed_cycles_;
        }
    }
    last_cycle_ms_ = now_ms;
}

void SystemHealthMonitor::reset() {
    last_cycle_ms_ = 0;
    missed_cycles_ = 0;
    watchdog_triggered_ = false;
}

bool SystemHealthMonitor::is_healthy() const {
    return !watchdog_triggered_ && missed_cycles_ < 3;
}

bool SystemHealthMonitor::watchdog_triggered() const {
    return watchdog_triggered_;
}

std::uint32_t SystemHealthMonitor::missed_cycles() const {
    return missed_cycles_;
}

std::uint64_t SystemHealthMonitor::last_cycle_ms() const {
    return last_cycle_ms_;
}

}  // namespace mili

#pragma once

#include "mili/platform/can_router.hpp"
#include "mili/decision_engine.hpp"
#include "mili/integration/control_publisher.hpp"
#include "mili/integration/integration_hub.hpp"
#include "mili/integration/product1_api.hpp"
#include "mili/integration/product2_api.hpp"
#include "mili/integration/timestamp_sync.hpp"
#include "mili/sensor_adapter.hpp"
#include "mili/types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace mili::hil {

struct HilCanEvent {
    std::uint64_t time_ms;
    std::uint16_t can_id;
    std::uint8_t data[8];
    std::uint8_t len;
};

struct HilRunResult {
    std::size_t events_processed;
    std::size_t cycles_executed;
    std::size_t mode_switch_count;
    bool synchronization_ok;
    bool control_frames_sent;
    OperationalMode final_mode;
    OperationalMode initial_mode;
    float final_confidence;
    std::uint64_t max_cycle_latency_us;
    std::uint64_t max_switch_path_latency_us;
};

// Synthetic HIL CAN ID for environmental injection (host replay only).
constexpr std::uint16_t kCanIdHilEnvironmental = 0x3F0;

struct HardwareMock {
    TimestampSynchronizer sync;
    PowerManagementClient product1;
    VioNavigationClient product2;
    SensorAdapter adapter;
    ControlSystemPublisher publisher;
    IntegrationHub hub;
    DecisionEngine engine;

    HardwareMock(CanSendCallback send_frame, void* user_data);
};

class HilRunner {
public:
    bool load_scenario_csv(const std::string& path);
    void add_event(const HilCanEvent& event);

    HilRunResult run(HardwareMock& mock, std::uint64_t cycle_step_ms = 100);

private:
    std::vector<HilCanEvent> events_{};

    void dispatch_event(HardwareMock& mock, const HilCanEvent& event);
};

}  // namespace mili::hil

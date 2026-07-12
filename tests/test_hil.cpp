#include "mili/hil/hil_runner.hpp"
#include "mili/integration/control_publisher.hpp"
#include "mili/integration/product1_api.hpp"
#include "mili/integration/product2_api.hpp"
#include "mili/integration/timestamp_sync.hpp"
#include "mili/protocol/messages.hpp"
#include "mili/schema/data_contract.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

bool capture_send(std::uint16_t can_id, const void* data, std::size_t len, void* user_data) {
    auto* frames = static_cast<std::vector<std::pair<std::uint16_t, std::size_t>>*>(user_data);
    frames->emplace_back(can_id, len);
    (void)data;
    return true;
}

void test_power_management_api() {
    mili::TimestampSynchronizer sync;
    mili::PowerManagementClient client(&sync);

    mili::protocol::Product1CanPayload payload{};
    payload.battery_level_x100 = 7250;
    payload.power_draw_x10 = 115;
    payload.flags = 0x11;
    payload.sequence = 1;
    payload.remaining_time_min = 42;
    payload.power_budget_x10 = 80;

    mili::schema::ContractHeader header{
        mili::schema::kContractVersion,
        static_cast<std::uint8_t>(mili::schema::DataSourceId::Product1Power),
        1,
        995,
    };
    client.on_status_frame(payload, header, 1000);

    check(std::fabs(client.battery_percent() - 72.5f) < 0.01f, "Power API battery");
    check(std::fabs(client.remaining_flight_time_min() - 42.0f) < 0.01f, "Power API flight time");
    check(client.is_connected(), "Power API connected");

    mili::Product1Data out{};
    check(client.poll(out, 1100), "Power API poll");
    check(out.schema_version == mili::schema::kContractVersion, "Power API schema version");
}

void test_vio_navigation_api() {
    mili::TimestampSynchronizer sync;
    mili::VioNavigationClient client(&sync);

    mili::protocol::Product2CanPayload nav{};
    nav.distance_home_x10 = 82;
    nav.velocity_x10 = 120;
    client.on_nav_frame(nav, 1000);

    mili::protocol::Product2VioExtPayload vio{};
    vio.heading_x10 = 1350;
    vio.altitude_m = 120;
    vio.vio_confidence_x100 = 92;
    vio.schema_version = mili::schema::kContractVersion;
    vio.sequence = 2;

    mili::schema::ContractHeader header{
        mili::schema::kContractVersion,
        static_cast<std::uint8_t>(mili::schema::DataSourceId::Product2Vio),
        2,
        997,
    };
    client.on_vio_frame(vio, header, 1000);

    check(std::fabs(client.distance_to_home_km() - 8.2f) < 0.01f, "VIO API distance");
    check(std::fabs(client.heading_deg() - 135.0f) < 0.01f, "VIO API heading");
    check(client.vio_confidence() > 0.9f, "VIO API confidence");
}

void test_timestamp_synchronizer() {
    mili::TimestampSynchronizer sync;
    sync.observe(mili::schema::DataSourceId::Product1Power, 1000, 1010);
    sync.observe(mili::schema::DataSourceId::Product1Power, 1100, 1112);
    sync.observe(mili::schema::DataSourceId::Product2Vio, 1000, 1005);
    sync.observe(mili::schema::DataSourceId::Product2Vio, 1100, 1107);

    check(sync.is_synchronized(1200), "Sources should be synchronized");
    check(std::fabs(sync.drift_ms(mili::schema::DataSourceId::Product1Power)) < 50.0f, "Drift bounded");
}

void test_control_system_publisher_sends_params() {
    std::vector<std::pair<std::uint16_t, std::size_t>> sent;
    mili::ControlSystemPublisher publisher(capture_send, &sent);

    mili::DecisionResult result{};
    result.selected_mode = mili::OperationalMode::Loiter;
    result.control_parameters.loiter_radius_km = 1.2f;
    result.control_parameters.return_urgency = 0.0f;
    result.control_parameters.speed_factor = 0.25f;

    check(publisher.publish(result), "Control publisher should publish");
    check(sent.size() == 2, "Should send mode command and control params");
    check(sent[0].first == mili::protocol::kCanIdModeCommand, "Mode command ID");
    check(sent[1].first == mili::protocol::kCanIdControlParams, "Control params ID");
    check(publisher.last_control_params().loiter_radius_x10 == 12, "Loiter radius encoded");
}

void test_hil_scenario_replay() {
    std::vector<std::pair<std::uint16_t, std::size_t>> sent;
    mili::hil::HardwareMock mock(capture_send, &sent);

    mili::hil::HilRunner runner;
    check(runner.load_scenario_csv("data/hil_scenarios/baseline.csv"), "HIL CSV should load");

    const auto result = runner.run(mock, 100);
    check(result.events_processed > 0, "HIL should process events");
    check(result.cycles_executed > 0, "HIL should run cycles");
    check(result.synchronization_ok, "HIL should synchronize sources");
    check(result.control_frames_sent, "HIL should publish control output");
    check(result.final_confidence > 0.0f, "HIL should produce confidence");
    check(sent.size() >= 2, "HIL should emit control frames");
}

}  // namespace

int main() {
    test_power_management_api();
    test_vio_navigation_api();
    test_timestamp_synchronizer();
    test_control_system_publisher_sends_params();
    test_hil_scenario_replay();

    if (failures > 0) {
        std::cerr << failures << " HIL/integration test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All HIL integration tests passed.\n";
    return EXIT_SUCCESS;
}

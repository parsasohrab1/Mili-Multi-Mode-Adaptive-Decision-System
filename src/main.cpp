#include "mili/decision_engine.hpp"
#include "mili/decision_loop.hpp"
#include "mili/integration/control_publisher.hpp"
#include "mili/integration/integration_hub.hpp"
#include "mili/integration/product1_api.hpp"
#include "mili/integration/product2_api.hpp"
#include "mili/integration/timestamp_sync.hpp"
#include "mili/mock_products.hpp"
#include "mili/protocol/messages.hpp"
#include "mili/schema/data_contract.hpp"
#include "mili/sensor_adapter.hpp"
#include "mili/types.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

namespace {

bool simulator_send(std::uint16_t can_id, const void*, std::size_t len, void* user_data) {
    auto* sent = static_cast<std::vector<std::pair<std::uint16_t, std::size_t>>*>(user_data);
    sent->emplace_back(can_id, len);
    return true;
}

}  // namespace

int main() {
    std::cout << "Mili Decision System - Product API Simulator\n";
    std::cout << "==============================================\n\n";

    std::vector<std::pair<std::uint16_t, std::size_t>> sent_frames;

    mili::TimestampSynchronizer sync;
    mili::PowerManagementClient product1(&sync);
    mili::VioNavigationClient product2(&sync);
    mili::SensorAdapter adapter;
    mili::ControlSystemPublisher publisher(simulator_send, &sent_frames);
    mili::IntegrationHub hub(adapter, publisher);

    hub.set_product1_client(&product1);
    hub.set_product2_client(&product2);

    mili::DecisionEngine engine(0);
    engine.load_config("config/default_weights.json");
    mili::DecisionLoop loop(engine, adapter, mili::kUpdateRateHz);

    mili::MockProduct1 mock_p1;
    mili::MockProduct2 mock_p2;

    const std::uint64_t step_ms = static_cast<std::uint64_t>(1000.0f / mili::kUpdateRateHz);
    std::uint64_t timestamp_ms = 0;

    for (int tick = 0; tick < 30; ++tick) {
        timestamp_ms += step_ms;

        const auto p1_sample = mock_p1.sample(timestamp_ms, 80.0f - tick * 1.5f);
        mili::protocol::Product1CanPayload p1_frame{};
        p1_frame.battery_level_x100 = static_cast<std::uint16_t>(p1_sample.battery_level_percent * 100.0f);
        p1_frame.power_draw_x10 = 120;
        p1_frame.flags = 0x10 | (p1_sample.low_power_warning ? 0x01 : 0);
        p1_frame.sequence = static_cast<std::uint8_t>(tick);
        p1_frame.remaining_time_min = static_cast<std::uint8_t>(60 - tick);
        p1_frame.power_budget_x10 = 80;

        mili::schema::ContractHeader p1_header{
            mili::schema::kContractVersion,
            static_cast<std::uint8_t>(mili::schema::DataSourceId::Product1Power),
            static_cast<std::uint16_t>(tick),
            static_cast<std::uint32_t>(timestamp_ms - 2),
        };
        product1.on_status_frame(p1_frame, p1_header, timestamp_ms);

        const auto p2_sample = mock_p2.sample(timestamp_ms, 4.0f + tick * 0.2f);
        mili::protocol::Product2CanPayload nav{};
        nav.position_x_dm = static_cast<std::int16_t>(p2_sample.position_x_km * 10.0f);
        nav.position_y_dm = static_cast<std::int16_t>(p2_sample.position_y_km * 10.0f);
        nav.distance_home_x10 = static_cast<std::uint16_t>(p2_sample.distance_to_home_km * 10.0f);
        nav.velocity_x10 = static_cast<std::uint16_t>(p2_sample.velocity_mps * 10.0f);
        product2.on_nav_frame(nav, timestamp_ms);

        mili::protocol::Product2VioExtPayload vio{};
        vio.heading_x10 = static_cast<std::uint16_t>(tick * 30);
        vio.altitude_m = 100;
        vio.vio_confidence_x100 = 90;
        vio.schema_version = mili::schema::kContractVersion;
        vio.sequence = static_cast<std::uint8_t>(tick);

        mili::schema::ContractHeader p2_header{
            mili::schema::kContractVersion,
            static_cast<std::uint8_t>(mili::schema::DataSourceId::Product2Vio),
            static_cast<std::uint16_t>(tick),
            static_cast<std::uint32_t>(timestamp_ms - 1),
        };
        product2.on_vio_frame(vio, p2_header, timestamp_ms);

        adapter.ingest_environmental(mili::mock_environmental(timestamp_ms), timestamp_ms);

        if (tick == 15) {
            mili::OperatorCommand command{};
            command.valid = true;
            command.has_priority_override = true;
            command.priority_override = mili::MissionPriority::Critical;
            adapter.ingest_operator(command, timestamp_ms);
        }

        hub.poll_inputs(timestamp_ms);
        const auto result = loop.tick(timestamp_ms);
        publisher.publish(result);

        if (tick % 10 == 0) {
            const auto input = adapter.to_sensor_input(timestamp_ms);
            std::cout << "Tick " << tick << " @ " << timestamp_ms << "ms\n";
            std::cout << "  Battery: " << input.battery_level << "%"
                      << " (budget=" << product1.power_budget_watts() << "W)\n";
            std::cout << "  Distance: " << input.distance_to_home_km << " km"
                      << " VIO conf=" << product2.vio_confidence() << "\n";
            std::cout << "  Sync: " << (sync.is_synchronized(timestamp_ms) ? "yes" : "no") << "\n";
            std::cout << "  Mode: " << mili::to_string(result.selected_mode)
                      << " (confidence=" << std::fixed << std::setprecision(3)
                      << result.confidence << ")\n";
            std::cout << "  Control: loiter_r="
                      << result.control_parameters.loiter_radius_km
                      << " return_u="
                      << result.control_parameters.return_urgency << "\n\n";
        }
    }

    const auto& stats = loop.stats();
    std::cout << "Loop ticks: " << stats.tick_count << "\n";
    std::cout << "Control frames sent: " << sent_frames.size() << "\n";
    std::cout << "Decision log entries: " << engine.logger().size() << "\n";
    std::cout << "Product API simulation complete.\n";

    return 0;
}

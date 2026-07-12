#include "mili/hil/hil_runner.hpp"

#include "mili/mock_products.hpp"
#include "mili/product_data.hpp"
#include "mili/protocol/messages.hpp"
#include "mili/schema/data_contract.hpp"

#include <chrono>
#include <cstring>
#include <fstream>
#include <sstream>

namespace mili::hil {

HardwareMock::HardwareMock(CanSendCallback send_frame, void* user_data)
    : product1(&sync)
    , product2(&sync)
    , publisher(send_frame, user_data)
    , hub(adapter, publisher)
    , engine(0) {
    hub.set_product1_client(&product1);
    hub.set_product2_client(&product2);
    engine.load_config("config/default_weights.json");
}

void HilRunner::add_event(const HilCanEvent& event) {
    events_.push_back(event);
}

bool HilRunner::load_scenario_csv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::stringstream stream(line);
        std::string cell;
        std::vector<std::string> fields;
        while (std::getline(stream, cell, ',')) {
            fields.push_back(cell);
        }
        if (fields.size() < 10) {
            continue;
        }

        HilCanEvent event{};
        event.time_ms = static_cast<std::uint64_t>(std::stoull(fields[0]));
        event.can_id = static_cast<std::uint16_t>(std::stoul(fields[1]));
        event.len = 8;
        for (std::size_t i = 0; i < 8; ++i) {
            event.data[i] = static_cast<std::uint8_t>(std::stoul(fields[2 + i]));
        }
        events_.push_back(event);
    }
    return !events_.empty();
}

void HilRunner::dispatch_event(HardwareMock& mock, const HilCanEvent& event) {
    if (event.len < 8) {
        return;
    }

    if (event.can_id == protocol::kCanIdProduct1Status) {
        protocol::Product1CanPayload payload{};
        std::memcpy(&payload, event.data, sizeof(payload));
        schema::ContractHeader header{
            schema::kContractVersion,
            static_cast<std::uint8_t>(schema::DataSourceId::Product1Power),
            payload.sequence,
            static_cast<std::uint32_t>(event.time_ms - 5),
        };
        mock.product1.on_status_frame(payload, header, event.time_ms);
        return;
    }

    if (event.can_id == protocol::kCanIdProduct2Nav) {
        protocol::Product2CanPayload payload{};
        std::memcpy(&payload, event.data, sizeof(payload));
        mock.product2.on_nav_frame(payload, event.time_ms);
        return;
    }

    if (event.can_id == protocol::kCanIdProduct2VioExt) {
        protocol::Product2VioExtPayload vio{};
        std::memcpy(&vio, event.data, sizeof(vio));
        schema::ContractHeader header{
            vio.schema_version,
            static_cast<std::uint8_t>(schema::DataSourceId::Product2Vio),
            vio.sequence,
            static_cast<std::uint32_t>(event.time_ms - 3),
        };
        mock.product2.on_vio_frame(vio, header, event.time_ms);
        return;
    }

    if (event.can_id == protocol::kCanIdOperatorCmd) {
        protocol::OperatorCanPayload payload{};
        std::memcpy(&payload, event.data, sizeof(payload));
        const auto command = protocol::decode_operator(payload, event.time_ms);
        mock.adapter.ingest_operator(command, event.time_ms);
        return;
    }

    if (event.can_id == kCanIdHilEnvironmental) {
        EnvironmentalData env = mock_environmental(event.time_ms);
        env.threat_level = static_cast<float>(event.data[0]) / 100.0f;
        env.comm_strength = static_cast<float>(event.data[1]) / 100.0f;
        env.target_visibility = static_cast<float>(event.data[2]) / 100.0f;
        env.wind_speed_mps = static_cast<float>(event.data[3]);
        env.valid = true;
        mock.adapter.ingest_environmental(env, event.time_ms);
    }
}

HilRunResult HilRunner::run(HardwareMock& mock, std::uint64_t cycle_step_ms) {
    HilRunResult result{};
    if (events_.empty()) {
        return result;
    }

    std::uint64_t next_cycle = events_.front().time_ms;
    std::size_t event_index = 0;
    bool first_cycle = true;

    while (event_index < events_.size() || next_cycle <= events_.back().time_ms) {
        while (event_index < events_.size() && events_[event_index].time_ms <= next_cycle) {
            dispatch_event(mock, events_[event_index]);
            ++event_index;
            ++result.events_processed;
        }

        const auto start = std::chrono::steady_clock::now();
        const auto decision = mock.hub.run_cycle(mock.engine, next_cycle);
        const auto end = std::chrono::steady_clock::now();

        const auto latency_us = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        if (latency_us > result.max_cycle_latency_us) {
            result.max_cycle_latency_us = latency_us;
        }

        if (first_cycle) {
            result.initial_mode = decision.selected_mode;
            first_cycle = false;
        }
        if (decision.mode_changed) {
            ++result.mode_switch_count;
            if (latency_us > result.max_switch_path_latency_us) {
                result.max_switch_path_latency_us = latency_us;
            }
        }

        result.final_mode = decision.selected_mode;
        result.final_confidence = decision.confidence;
        ++result.cycles_executed;

        if (event_index >= events_.size()) {
            break;
        }
        next_cycle += cycle_step_ms;
    }

    result.synchronization_ok = mock.sync.is_synchronized(next_cycle);
    result.control_frames_sent = mock.publisher.publish_count() > 0;
    return result;
}

}  // namespace mili::hil

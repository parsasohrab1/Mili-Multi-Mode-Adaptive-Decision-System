#pragma once

#include "mili/data_quality_monitor.hpp"
#include "mili/decision_logger.hpp"
#include "mili/decision_logger_interface.hpp"
#include "mili/embedded_config.hpp"
#include "mili/hysteresis_controller.hpp"
#include "mili/mode_scorer.hpp"
#include "mili/mode_state_machine.hpp"
#include "mili/ring_buffer_logger.hpp"
#include "mili/sensor_noise_model.hpp"
#include "mili/sensor_imputer.hpp"
#include "mili/topsis_scorer.hpp"
#include "mili/types.hpp"
#include "mili/weight_manager.hpp"

#include <cstdint>
#include <string>

namespace mili {

struct EngineComponents {
    ModeScorer* scorer = nullptr;
    McdmEngine* mcdm_engine = nullptr;
    HysteresisController* hysteresis = nullptr;
    IDecisionLogger* logger = nullptr;
    WeightManager* weight_manager = nullptr;
    ModeStateMachine* state_machine = nullptr;
};

struct EvaluateContext {
    const DataQualityStatus* quality = nullptr;
    const ImputationWeights* imputation = nullptr;
    bool quality_emergency = false;
};

class DecisionEngine {
public:
    explicit DecisionEngine(std::uint64_t initial_timestamp_ms = 0);
    DecisionEngine(const EngineComponents& components, std::uint64_t initial_timestamp_ms = 0);

    DecisionResult evaluate(const SensorInput& input, std::uint64_t timestamp_ms);
    DecisionResult evaluate(
        const SensorInput& input,
        std::uint64_t timestamp_ms,
        const EvaluateContext& context);

    void set_mcdm_enabled(bool enabled);
    void set_noise_enabled(bool enabled);
    bool mcdm_enabled() const;
    bool noise_enabled() const;

    bool load_config(const std::string& path);
    bool reload_config();
    bool set_mode_weight(OperationalMode mode, const char* factor_name, float value);

    WeightManager& weight_manager();
    const WeightManager& weight_manager() const;

    ModeScorer& scorer();
    const ModeScorer& scorer() const;
    McdmEngine& mcdm_engine();
    const McdmEngine& mcdm_engine() const;
    ModeStateMachine& state_machine();
    const ModeStateMachine& state_machine() const;

    void set_hysteresis_seconds(float seconds);
    float hysteresis_seconds() const;

    IDecisionLogger& logger();
    const IDecisionLogger& logger() const;

    OperationalMode current_mode() const;
    float last_confidence() const;

private:
    WeightManager owned_weight_manager_;
    ModeScorer owned_scorer_;
    McdmEngine owned_mcdm_engine_;
    HysteresisController owned_hysteresis_;
#ifdef MILI_EMBEDDED
    EmbeddedDecisionLogger owned_logger_;
#else
    DecisionLogger owned_logger_;
#endif
    ModeStateMachine owned_state_machine_;
    SensorNoiseModel noise_model_;

    WeightManager* weight_manager_;
    ModeScorer* scorer_;
    McdmEngine* mcdm_engine_;
    HysteresisController* hysteresis_;
    IDecisionLogger* logger_;
    ModeStateMachine* state_machine_;

    OperationalMode current_mode_;
    std::uint64_t last_timestamp_ms_;
    float last_confidence_;
    bool mcdm_enabled_;
    bool noise_enabled_;

    std::array<float, kModeCount> compute_mode_scores(const SensorInput& input) const;
};

}  // namespace mili

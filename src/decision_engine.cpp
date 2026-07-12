#include "mili/decision_engine.hpp"

#include "mili/confidence.hpp"
#include "mili/low_confidence_policy.hpp"
#include "mili/mode_parameters.hpp"
#include "mili/sensor_imputer.hpp"

namespace mili {

DecisionEngine::DecisionEngine(std::uint64_t initial_timestamp_ms)
    : DecisionEngine(EngineComponents{}, initial_timestamp_ms) {}

DecisionEngine::DecisionEngine(const EngineComponents& components, std::uint64_t initial_timestamp_ms)
    : owned_scorer_(components.weight_manager != nullptr ? components.weight_manager : &owned_weight_manager_)
    , owned_mcdm_engine_(components.weight_manager != nullptr ? components.weight_manager : &owned_weight_manager_)
    , owned_hysteresis_()
    , owned_logger_()
    , owned_state_machine_()
    , weight_manager_(components.weight_manager != nullptr ? components.weight_manager : &owned_weight_manager_)
    , scorer_(components.scorer != nullptr ? components.scorer : &owned_scorer_)
    , mcdm_engine_(components.mcdm_engine != nullptr ? components.mcdm_engine : &owned_mcdm_engine_)
    , hysteresis_(components.hysteresis != nullptr ? components.hysteresis : &owned_hysteresis_)
    , logger_(components.logger != nullptr ? components.logger : &owned_logger_)
    , state_machine_(components.state_machine != nullptr ? components.state_machine : &owned_state_machine_)
    , current_mode_(OperationalMode::Reconnaissance)
    , last_timestamp_ms_(initial_timestamp_ms)
    , last_confidence_(0.7f)
    , mcdm_enabled_(true)
    , noise_enabled_(true) {
    if (components.hysteresis == nullptr) {
        hysteresis_->set_hysteresis_seconds(weight_manager_->config().hysteresis_seconds);
    }
    state_machine_->apply_state(ModeStateMachine::mode_to_state(current_mode_));
}

std::array<float, kModeCount> DecisionEngine::compute_mode_scores(const SensorInput& input) const {
    if (mcdm_enabled_) {
        return mcdm_engine_->compute_scores(input);
    }
    return scorer_->compute_scores(input);
}

DecisionResult DecisionEngine::evaluate(const SensorInput& input, std::uint64_t timestamp_ms) {
    return evaluate(input, timestamp_ms, EvaluateContext{});
}

DecisionResult DecisionEngine::evaluate(
    const SensorInput& input, std::uint64_t timestamp_ms, const EvaluateContext& context) {
    const bool quality_emergency = context.quality_emergency
        || (context.quality != nullptr && context.quality->emergency_return);

    const auto noisy_input = noise_enabled_
        ? noise_model_.apply(input, last_confidence_, static_cast<std::uint32_t>(timestamp_ms))
        : input;

    auto scores = compute_mode_scores(noisy_input);
    if (context.imputation != nullptr) {
        SensorImputer::scale_scores(scores, *context.imputation);
    } else if (context.quality != nullptr) {
        ImputationWeights weights{};
        weights.combined = context.quality->imputation_factor;
        weights.battery = 1.0f;
        weights.navigation = 1.0f;
        weights.environmental = 1.0f;
        SensorImputer::scale_scores(scores, weights);
    }

    auto candidate = select_best_mode(scores);
    const auto confidence = compute_confidence(scores);
    last_confidence_ = confidence;

    const auto transition = state_machine_->evaluate_transition(
        candidate, noisy_input, confidence, quality_emergency);

    OperationalMode target_mode = candidate;
    if (transition.is_emergency || quality_emergency) {
        target_mode = OperationalMode::ReturnHome;
        state_machine_->apply_state(FsmState::Emergency);
    } else {
        target_mode = apply_low_confidence_policy(target_mode, confidence);
        if (transition.allowed) {
            state_machine_->apply_state(transition.next_state);
        } else {
            target_mode = current_mode_;
        }
    }

    bool mode_changed = false;
    if (hysteresis_->should_switch(current_mode_, target_mode, timestamp_ms)) {
        current_mode_ = target_mode;
        mode_changed = true;
    }

    DecisionResult result{};
    result.selected_mode = current_mode_;
    result.selected_score = scores[static_cast<std::size_t>(current_mode_)];
    result.confidence = confidence;
    result.low_confidence = confidence < kLowConfidenceThreshold;
    result.degraded_inputs = context.quality != nullptr && context.quality->degraded_inputs;
    result.emergency_forced = quality_emergency;
    result.control_parameters = ModeParameterMapper::for_mode(
        current_mode_, noisy_input, result.selected_score);
    apply_low_confidence_parameters(result.control_parameters, confidence);
    result.mode_changed = mode_changed;
    result.timestamp_ms = timestamp_ms;

    for (std::size_t i = 0; i < kModeCount; ++i) {
        result.all_scores[i] = ModeScore{
            static_cast<OperationalMode>(i),
            scores[i]
        };
    }

    logger_->log(noisy_input, result, timestamp_ms);
    last_timestamp_ms_ = timestamp_ms;

    return result;
}

void DecisionEngine::set_mcdm_enabled(bool enabled) {
    mcdm_enabled_ = enabled;
}

void DecisionEngine::set_noise_enabled(bool enabled) {
    noise_enabled_ = enabled;
}

bool DecisionEngine::mcdm_enabled() const {
    return mcdm_enabled_;
}

bool DecisionEngine::noise_enabled() const {
    return noise_enabled_;
}

bool DecisionEngine::load_config(const std::string& path) {
    const bool loaded = weight_manager_->load_from_file(path);
    if (loaded) {
        hysteresis_->set_hysteresis_seconds(weight_manager_->config().hysteresis_seconds);
    }
    return loaded;
}

bool DecisionEngine::reload_config() {
    const bool loaded = weight_manager_->reload();
    if (loaded) {
        hysteresis_->set_hysteresis_seconds(weight_manager_->config().hysteresis_seconds);
    }
    return loaded;
}

bool DecisionEngine::set_mode_weight(OperationalMode mode, const char* factor_name, float value) {
    return weight_manager_->set_mode_factor(mode, factor_name, value);
}

WeightManager& DecisionEngine::weight_manager() {
    return *weight_manager_;
}

const WeightManager& DecisionEngine::weight_manager() const {
    return *weight_manager_;
}

ModeScorer& DecisionEngine::scorer() {
    return *scorer_;
}

const ModeScorer& DecisionEngine::scorer() const {
    return *scorer_;
}

McdmEngine& DecisionEngine::mcdm_engine() {
    return *mcdm_engine_;
}

const McdmEngine& DecisionEngine::mcdm_engine() const {
    return *mcdm_engine_;
}

ModeStateMachine& DecisionEngine::state_machine() {
    return *state_machine_;
}

const ModeStateMachine& DecisionEngine::state_machine() const {
    return *state_machine_;
}

void DecisionEngine::set_hysteresis_seconds(float seconds) {
    hysteresis_->set_hysteresis_seconds(seconds);
}

float DecisionEngine::hysteresis_seconds() const {
    return hysteresis_->hysteresis_seconds();
}

const IDecisionLogger& DecisionEngine::logger() const {
    return *logger_;
}

IDecisionLogger& DecisionEngine::logger() {
    return *logger_;
}

OperationalMode DecisionEngine::current_mode() const {
    return current_mode_;
}

float DecisionEngine::last_confidence() const {
    return last_confidence_;
}

}  // namespace mili

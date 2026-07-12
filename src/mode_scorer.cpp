#include "mili/mode_scorer.hpp"

#include "mili/factor_normalizer.hpp"

namespace mili {

namespace {

float score_from_terms(const float* weights, const float* factors, std::size_t count) {
    return FactorNormalizer::weighted_sum(weights, factors, count);
}

}  // namespace

ModeScorer::ModeScorer(const WeightManager* weight_manager)
    : weight_manager_(weight_manager) {}

const WeightConfig& ModeScorer::weights() const {
    if (weight_manager_ != nullptr) {
        return weight_manager_->config();
    }

    static const WeightConfig kFallback = make_default_weight_config();
    return kFallback;
}

float ModeScorer::score_reconnaissance(const NormalizedFactors& f) const {
    const auto& w = weights().reconnaissance;
    const float factors[] = {f.battery, f.threat_inverse, f.comm, f.distance_inverse};
    return score_from_terms(w.terms, factors, 4);
}

float ModeScorer::score_surveillance(const NormalizedFactors& f) const {
    const auto& w = weights().surveillance;
    const float factors[] = {f.visibility, f.threat_inverse, f.battery, f.wind_inverse};
    return score_from_terms(w.terms, factors, 4);
}

float ModeScorer::score_pursuit(const NormalizedFactors& f) const {
    const auto& w = weights().pursuit;
    const float threat_half_inv = 1.0f - f.threat * 0.5f;
    const float factors[] = {f.visibility, f.battery_ok_40, threat_half_inv, f.comm_ok};
    return score_from_terms(w.terms, factors, 4);
}

float ModeScorer::score_loiter(const NormalizedFactors& f) const {
    const auto& w = weights().loiter;
    const float factors[] = {f.comm_inverse, f.threat_high_70, f.visibility_inverse, f.battery};
    return score_from_terms(w.terms, factors, 4);
}

float ModeScorer::score_return_home(const NormalizedFactors& f) const {
    const auto& w = weights().return_home;
    const float factors[] = {f.battery_inverse, f.threat, f.distance_close, f.comm_inverse};
    return score_from_terms(w.terms, factors, 4);
}

float ModeScorer::score_engagement(const NormalizedFactors& f) const {
    const auto& w = weights().engagement;
    const float term_weights[] = {
        w.target_visibility,
        w.threat_threshold,
        w.battery_threshold,
        w.priority_boost
    };
    const float factors[] = {f.visibility, f.threat_high_60, f.battery_ok_50, f.priority_critical};
    return score_from_terms(term_weights, factors, 4) - f.distance_km * w.distance_penalty;
}

void ModeScorer::apply_priority_multipliers(
    MissionPriority priority, std::array<float, kModeCount>& scores) const {
    const auto& cfg = weights();
    if (priority == MissionPriority::Critical) {
        scores[static_cast<std::size_t>(OperationalMode::Engagement)] *= cfg.critical.engagement;
        scores[static_cast<std::size_t>(OperationalMode::Surveillance)] *= cfg.critical.surveillance;
    } else if (priority == MissionPriority::Low) {
        scores[static_cast<std::size_t>(OperationalMode::Loiter)] *= cfg.low.loiter;
        scores[static_cast<std::size_t>(OperationalMode::ReturnHome)] *= cfg.low.return_home;
        scores[static_cast<std::size_t>(OperationalMode::Reconnaissance)] *= cfg.low.reconnaissance;
    }
}

void ModeScorer::apply_environmental_factors(
    const SensorInput& input, std::array<float, kModeCount>& scores) const {
    const auto& env = weights().environmental;

    if (input.weather == Weather::Rain) {
        scores[static_cast<std::size_t>(OperationalMode::Surveillance)] *= env.rain_surveillance;
        scores[static_cast<std::size_t>(OperationalMode::Pursuit)] *= env.rain_pursuit;
    } else if (input.weather == Weather::Fog) {
        scores[static_cast<std::size_t>(OperationalMode::Reconnaissance)] *= env.fog_reconnaissance;
        scores[static_cast<std::size_t>(OperationalMode::Surveillance)] *= env.fog_surveillance;
    }

    if (input.time_of_day == TimeOfDay::Night) {
        scores[static_cast<std::size_t>(OperationalMode::Reconnaissance)] *= env.night_reconnaissance;
        scores[static_cast<std::size_t>(OperationalMode::Surveillance)] *= env.night_surveillance;
    }
}

std::array<float, kModeCount> ModeScorer::compute_scores(const SensorInput& input) const {
    const auto factors = FactorNormalizer::normalize(input);

    std::array<float, kModeCount> scores{};
    scores[static_cast<std::size_t>(OperationalMode::Reconnaissance)] = score_reconnaissance(factors);
    scores[static_cast<std::size_t>(OperationalMode::Surveillance)] = score_surveillance(factors);
    scores[static_cast<std::size_t>(OperationalMode::Pursuit)] = score_pursuit(factors);
    scores[static_cast<std::size_t>(OperationalMode::Loiter)] = score_loiter(factors);
    scores[static_cast<std::size_t>(OperationalMode::ReturnHome)] = score_return_home(factors);
    scores[static_cast<std::size_t>(OperationalMode::Engagement)] = score_engagement(factors);

    apply_priority_multipliers(input.mission_priority, scores);
    apply_environmental_factors(input, scores);

    return scores;
}

}  // namespace mili

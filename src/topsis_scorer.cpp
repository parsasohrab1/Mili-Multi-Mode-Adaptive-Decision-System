#include "mili/topsis_scorer.hpp"

#include "mili/mode_scorer.hpp"

#include <algorithm>
#include <cmath>

namespace mili {

TopsisScorer::TopsisScorer(const WeightManager* weight_manager)
    : weight_manager_(weight_manager) {}

std::array<float, TopsisScorer::kCriteriaCount> TopsisScorer::criteria_weights() const {
    return {0.20f, 0.20f, 0.20f, 0.15f, 0.15f, 0.10f};
}

TopsisScorer::DecisionMatrix TopsisScorer::build_decision_matrix(const NormalizedFactors& f) const {
    DecisionMatrix matrix{};

    const std::array<std::array<float, kCriteriaCount>, kModeCount> rows = {{
        {f.battery, f.threat_inverse, f.comm, f.distance_inverse, f.visibility, f.wind_inverse},
        {f.visibility, f.threat_inverse, f.battery, f.wind_inverse, f.comm, f.distance_inverse},
        {f.visibility, f.battery_ok_40, f.threat_inverse, f.comm_ok, f.battery, f.threat},
        {f.comm_inverse, f.threat_high_70, f.visibility_inverse, f.battery, f.threat, f.comm},
        {f.battery_inverse, f.threat, f.distance_close, f.comm_inverse, f.battery, f.distance_inverse},
        {f.visibility, f.threat_high_60, f.battery_ok_50, f.priority_critical, f.threat, f.distance_inverse}
    }};

    for (std::size_t mode = 0; mode < kModeCount; ++mode) {
        matrix.values[mode] = rows[mode];
    }
    return matrix;
}

float TopsisScorer::column_norm(const DecisionMatrix& matrix, std::size_t criterion) {
    float sum_sq = 0.0f;
    for (std::size_t mode = 0; mode < kModeCount; ++mode) {
        const float value = matrix.values[mode][criterion];
        sum_sq += value * value;
    }
    return std::sqrt(sum_sq);
}

TopsisResult TopsisScorer::run_topsis(const DecisionMatrix& matrix, const float* weights) {
    constexpr std::size_t kCriteria = TopsisScorer::kCriteriaCount;
    TopsisResult result{};
    std::array<std::array<float, kCriteria>, kModeCount> weighted{};

    for (std::size_t c = 0; c < kCriteria; ++c) {
        const float norm = column_norm(matrix, c);
        for (std::size_t mode = 0; mode < kModeCount; ++mode) {
            const float r_ij = norm > 0.0f ? matrix.values[mode][c] / norm : 0.0f;
            weighted[mode][c] = weights[c] * r_ij;
        }
    }

    std::array<float, kCriteria> ideal{};
    std::array<float, kCriteria> anti_ideal{};
    for (std::size_t c = 0; c < kCriteria; ++c) {
        float max_v = weighted[0][c];
        float min_v = weighted[0][c];
        for (std::size_t mode = 1; mode < kModeCount; ++mode) {
            max_v = std::max(max_v, weighted[mode][c]);
            min_v = std::min(min_v, weighted[mode][c]);
        }
        ideal[c] = max_v;
        anti_ideal[c] = min_v;
    }

    for (std::size_t mode = 0; mode < kModeCount; ++mode) {
        float d_plus = 0.0f;
        float d_minus = 0.0f;
        for (std::size_t c = 0; c < kCriteria; ++c) {
            const float plus = weighted[mode][c] - ideal[c];
            const float minus = weighted[mode][c] - anti_ideal[c];
            d_plus += plus * plus;
            d_minus += minus * minus;
        }
        result.distance_positive[mode] = std::sqrt(d_plus);
        result.distance_negative[mode] = std::sqrt(d_minus);
        const float denom = result.distance_positive[mode] + result.distance_negative[mode];
        result.closeness[mode] = denom > 0.0f
            ? result.distance_negative[mode] / denom
            : 0.0f;
    }

    return result;
}

TopsisResult TopsisScorer::analyze(const SensorInput& input) const {
    const auto factors = FactorNormalizer::normalize(input);
    const auto matrix = build_decision_matrix(factors);
    const auto weights = criteria_weights();
    return run_topsis(matrix, weights.data());
}

std::array<float, kModeCount> TopsisScorer::compute_scores(const SensorInput& input) const {
    return analyze(input).closeness;
}

McdmEngine::McdmEngine(const WeightManager* weight_manager)
    : weight_manager_(weight_manager)
    , topsis_(weight_manager)
    , objectives_() {}

std::array<float, kModeCount> McdmEngine::compute_scores(const SensorInput& input) const {
    last_topsis_ = topsis_.analyze(input);
    last_objective_ = objectives_.score_all_modes(input);

    ModeScorer weighted(weight_manager_);
    const auto weighted_scores = weighted.compute_scores(input);

    std::array<float, kModeCount> combined{};
    for (std::size_t i = 0; i < kModeCount; ++i) {
        combined[i] = 0.30f * last_topsis_.closeness[i]
                    + 0.20f * last_objective_[i]
                    + 0.50f * weighted_scores[i];
    }
    return combined;
}

const TopsisResult& McdmEngine::last_topsis() const {
    return last_topsis_;
}

const std::array<float, kModeCount>& McdmEngine::last_objective_scores() const {
    return last_objective_;
}

}  // namespace mili

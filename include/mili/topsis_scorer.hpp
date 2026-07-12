#pragma once

#include "mili/factor_normalizer.hpp"
#include "mili/multi_objective.hpp"
#include "mili/types.hpp"
#include "mili/weight_manager.hpp"

#include <array>

namespace mili {

struct TopsisResult {
    std::array<float, kModeCount> closeness{};
    std::array<float, kModeCount> distance_positive{};
    std::array<float, kModeCount> distance_negative{};
};

class TopsisScorer {
public:
    static constexpr std::size_t kCriteriaCount = 6;

    explicit TopsisScorer(const WeightManager* weight_manager = nullptr);

    std::array<float, kModeCount> compute_scores(const SensorInput& input) const;
    TopsisResult analyze(const SensorInput& input) const;

private:
    const WeightManager* weight_manager_;

    struct DecisionMatrix {
        std::array<std::array<float, kCriteriaCount>, kModeCount> values{};
    };

    std::array<float, kCriteriaCount> criteria_weights() const;
    DecisionMatrix build_decision_matrix(const NormalizedFactors& factors) const;
    static float column_norm(const DecisionMatrix& matrix, std::size_t criterion);
    static TopsisResult run_topsis(const DecisionMatrix& matrix, const float* weights);
};

class McdmEngine {
public:
    explicit McdmEngine(const WeightManager* weight_manager = nullptr);

    std::array<float, kModeCount> compute_scores(const SensorInput& input) const;

    const TopsisResult& last_topsis() const;
    const std::array<float, kModeCount>& last_objective_scores() const;

private:
    const WeightManager* weight_manager_;
    TopsisScorer topsis_;
    MultiObjectiveEvaluator objectives_;
    mutable TopsisResult last_topsis_{};
    mutable std::array<float, kModeCount> last_objective_{};
};

}  // namespace mili

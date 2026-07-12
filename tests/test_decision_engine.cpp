#include "mili/confidence.hpp"
#include "mili/decision_engine.hpp"
#include "mili/factor_normalizer.hpp"
#include "mili/hysteresis_controller.hpp"
#include "mili/mode_parameters.hpp"
#include "mili/mode_scorer.hpp"
#include "mili/types.hpp"

#include "csv_scenario.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void check_near(float actual, float expected, float epsilon, const char* message) {
    if (std::fabs(actual - expected) > epsilon) {
        std::cerr << "FAIL: " << message << " (actual=" << actual << ", expected=" << expected << ")\n";
        ++failures;
    }
}

mili::SensorInput make_reconnaissance_favored() {
    mili::SensorInput input{};
    input.battery_level = 95.0f;
    input.threat_level = 0.05f;
    input.comm_strength = 0.95f;
    input.target_visibility = 0.4f;
    input.distance_to_home_km = 2.0f;
    input.mission_priority = mili::MissionPriority::High;
    return input;
}

mili::SensorInput make_surveillance_favored() {
    mili::SensorInput input{};
    input.battery_level = 35.0f;
    input.threat_level = 0.05f;
    input.target_visibility = 0.88f;
    input.wind_speed_mps = 0.5f;
    input.comm_strength = 0.3f;
    input.distance_to_home_km = 10.0f;
    return input;
}

mili::SensorInput make_pursuit_favored() {
    mili::SensorInput input{};
    input.battery_level = 75.0f;
    input.threat_level = 0.2f;
    input.target_visibility = 0.9f;
    input.comm_strength = 0.8f;
    return input;
}

mili::SensorInput make_loiter_favored() {
    mili::SensorInput input{};
    input.comm_strength = 0.1f;
    input.threat_level = 0.85f;
    input.target_visibility = 0.1f;
    input.battery_level = 60.0f;
    input.distance_to_home_km = 15.0f;
    input.mission_priority = mili::MissionPriority::Low;
    return input;
}

mili::SensorInput make_return_home_favored() {
    mili::SensorInput input{};
    input.battery_level = 12.0f;
    input.threat_level = 0.85f;
    input.comm_strength = 0.2f;
    input.distance_to_home_km = 1.0f;
    return input;
}

mili::SensorInput make_engagement_favored() {
    mili::SensorInput input{};
    input.battery_level = 80.0f;
    input.threat_level = 0.9f;
    input.target_visibility = 0.95f;
    input.mission_priority = mili::MissionPriority::Critical;
    input.distance_to_home_km = 4.0f;
    return input;
}

void test_factor_normalization() {
    mili::SensorInput input{};
    input.battery_level = 150.0f;
    input.threat_level = -0.5f;
    input.comm_strength = 2.0f;
    input.distance_to_home_km = 50.0f;
    input.wind_speed_mps = 30.0f;

    const auto factors = mili::FactorNormalizer::normalize(input);
    check_near(factors.battery, 1.0f, 0.001f, "Battery should clamp to 1");
    check_near(factors.threat, 0.0f, 0.001f, "Threat should clamp to 0");
    check_near(factors.comm, 1.0f, 0.001f, "Comm should clamp to 1");
    check_near(factors.distance_inverse, 0.0f, 0.001f, "Distance inverse should clamp to 0");
    check_near(factors.wind_inverse, 0.0f, 0.001f, "Wind inverse should clamp to 0");
}

void test_mode_scorer_all_modes() {
    mili::ModeScorer scorer;

    struct Case {
        mili::SensorInput input;
        mili::OperationalMode expected;
        const char* label;
    };

    const Case cases[] = {
        {make_reconnaissance_favored(), mili::OperationalMode::Reconnaissance, "reconnaissance"},
        {make_surveillance_favored(), mili::OperationalMode::Surveillance, "surveillance"},
        {make_pursuit_favored(), mili::OperationalMode::Pursuit, "pursuit"},
        {make_loiter_favored(), mili::OperationalMode::Loiter, "loiter"},
        {make_return_home_favored(), mili::OperationalMode::ReturnHome, "return_home"},
        {make_engagement_favored(), mili::OperationalMode::Engagement, "engagement"},
    };

    for (const auto& test_case : cases) {
        const auto scores = scorer.compute_scores(test_case.input);
        const auto best = mili::select_best_mode(scores);
        check(best == test_case.expected, test_case.label);
    }
}

void test_mode_scorer_edge_cases() {
    mili::ModeScorer scorer;

    mili::SensorInput all_zero{};
    all_zero.battery_level = 0.0f;
    all_zero.threat_level = 0.0f;
    all_zero.comm_strength = 0.0f;
    all_zero.target_visibility = 0.0f;
    const auto zero_scores = scorer.compute_scores(all_zero);
    check(std::isfinite(zero_scores[0]), "Scores should be finite for zero input");

    mili::SensorInput rain_night = make_surveillance_favored();
    rain_night.weather = mili::Weather::Rain;
    rain_night.time_of_day = mili::TimeOfDay::Night;
    const auto degraded = scorer.compute_scores(rain_night);
    const auto clear_day = scorer.compute_scores(make_surveillance_favored());
    check(
        degraded[static_cast<std::size_t>(mili::OperationalMode::Surveillance)]
            < clear_day[static_cast<std::size_t>(mili::OperationalMode::Surveillance)],
        "Rain and night should reduce surveillance score");
}

void test_confidence_and_parameters() {
    mili::ModeScorer scorer;
    const auto input = make_engagement_favored();
    const auto scores = scorer.compute_scores(input);
    const auto confidence = mili::compute_confidence(scores);

    check(confidence >= 0.3f && confidence <= 0.99f, "Confidence in valid range");

    const auto params = mili::ModeParameterMapper::for_mode(
        mili::OperationalMode::Engagement, input, scores[static_cast<std::size_t>(mili::OperationalMode::Engagement)]);
    check(params.engagement_readiness > 0.5f, "Engagement mode should set readiness");
    check(params.speed_factor > 0.5f, "Engagement mode should set speed factor");
}

void test_hysteresis() {
    mili::HysteresisController hysteresis(5.0f);

    check(
        !hysteresis.should_switch(
            mili::OperationalMode::Reconnaissance,
            mili::OperationalMode::Surveillance,
            0),
        "Should not switch immediately");

    check(
        !hysteresis.should_switch(
            mili::OperationalMode::Reconnaissance,
            mili::OperationalMode::Surveillance,
            3000),
        "Should not switch before 5 seconds");

    check(
        hysteresis.should_switch(
            mili::OperationalMode::Reconnaissance,
            mili::OperationalMode::Surveillance,
            5000),
        "Should switch after 5 seconds");
}

void test_hysteresis_stability_30s() {
    mili::DecisionEngine engine(0);
    engine.set_hysteresis_seconds(5.0f);

    const auto input_a = make_surveillance_favored();
    const auto input_b = make_return_home_favored();

    int switch_count = 0;
    mili::OperationalMode previous = engine.current_mode();
    const std::uint64_t step_ms = static_cast<std::uint64_t>(1000.0f / mili::kUpdateRateHz);
    const int cycles = static_cast<int>(30.0f * mili::kUpdateRateHz);

    for (int i = 0; i < cycles; ++i) {
        const auto& input = (i % 20 < 10) ? input_a : input_b;
        const std::uint64_t timestamp = static_cast<std::uint64_t>(i) * step_ms;
        const auto result = engine.evaluate(input, timestamp);
        if (result.mode_changed) {
            ++switch_count;
        }
        if (result.selected_mode != previous && !result.mode_changed) {
            check(false, "Mode changed without mode_changed flag");
        }
        previous = result.selected_mode;
    }

    check(switch_count <= 6, "Hysteresis should prevent rapid oscillation over 30 seconds");
}

void test_decision_engine_logging() {
    mili::DecisionEngine engine;

    mili::SensorInput input{};
    input.battery_level = 90.0f;
    input.threat_level = 0.1f;
    input.comm_strength = 0.9f;
    input.target_visibility = 0.8f;

    const auto result = engine.evaluate(input, 0);
    check(result.control_parameters.speed_factor > 0.0f, "Control parameters should be populated");
    check(engine.logger().size() == 1, "Decision should be logged");

    for (int i = 0; i < 1001; ++i) {
        engine.evaluate(input, static_cast<std::uint64_t>(i * 100));
    }
    check(engine.logger().size() == mili::kMaxLogEntries, "Logger should cap at 1000 entries");
}

void test_benchmark_evaluate() {
    mili::DecisionEngine engine;
    const auto input = make_engagement_favored();

    constexpr int iterations = 1000;
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        engine.evaluate(input, static_cast<std::uint64_t>(i));
    }
    const auto end = std::chrono::steady_clock::now();

    const auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    const double avg_ms = static_cast<double>(total_us) / iterations / 1000.0;

    std::cout << "Benchmark: evaluate() average = " << avg_ms << " ms\n";
    check(avg_ms < 5.0, "evaluate() average time should be below 5ms");
}

void test_benchmark_switch_latency() {
    mili::DecisionEngine engine(0);
    engine.set_hysteresis_seconds(5.0f);

    const auto input = make_return_home_favored();
    engine.evaluate(make_reconnaissance_favored(), 0);

    bool switched = false;
    for (int i = 1; i <= 51; ++i) {
        const auto result = engine.evaluate(input, static_cast<std::uint64_t>(i * 100));
        if (result.mode_changed) {
            switched = true;
            break;
        }
    }

    std::cout << "Benchmark: switch completed=" << (switched ? "yes" : "no") << "\n";
    check(switched, "Mode should switch after 5 second hysteresis period");
}

void test_csv_validation(const std::string& csv_path) {
    const auto scenarios = mili::test::load_csv_scenarios(csv_path);
    check(scenarios.size() >= 400, "CSV should contain at least 400 scenarios");

    mili::ModeScorer scorer;
    int mode_matches = 0;
    int score_matches = 0;

    for (const auto& scenario : scenarios) {
        const auto scores = scorer.compute_scores(scenario.input);
        const auto best = mili::select_best_mode(scores);

        if (mili::to_string(best) == scenario.expected_mode) {
            ++mode_matches;
        }

        bool row_score_match = true;
        for (std::size_t i = 0; i < mili::kModeCount; ++i) {
            if (std::fabs(scores[i] - scenario.expected_scores[i]) > 0.02f) {
                row_score_match = false;
                break;
            }
        }
        if (row_score_match) {
            ++score_matches;
        }
    }

    const double mode_accuracy = 100.0 * mode_matches / scenarios.size();
    const double score_accuracy = 100.0 * score_matches / scenarios.size();

    std::cout << "CSV validation: mode accuracy = " << mode_accuracy << "%\n";
    std::cout << "CSV validation: score accuracy = " << score_accuracy << "%\n";

    check(mode_accuracy >= 92.0, "Mode selection accuracy should be at least 92%");
    check(score_accuracy >= 95.0, "Score accuracy should be at least 95%");
}

void print_usage() {
    std::cout << "Usage: mili_tests [options]\n"
              << "  --mode-scorer\n"
              << "  --hysteresis\n"
              << "  --decision-engine\n"
              << "  --benchmark\n"
              << "  --csv-validation [path]\n"
              << "  --all\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    bool run_scorer = false;
    bool run_hysteresis = false;
    bool run_engine = false;
    bool run_benchmark = false;
    bool run_csv = false;
    std::string csv_path = "data/mission_decision_data.csv";

    if (argc < 2) {
        run_scorer = run_hysteresis = run_engine = run_benchmark = run_csv = true;
    } else {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--mode-scorer") run_scorer = true;
            else if (arg == "--hysteresis") run_hysteresis = true;
            else if (arg == "--decision-engine") run_engine = true;
            else if (arg == "--benchmark") run_benchmark = true;
            else if (arg == "--csv-validation") {
                run_csv = true;
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    csv_path = argv[++i];
                }
            } else if (arg == "--all") {
                run_scorer = run_hysteresis = run_engine = run_benchmark = run_csv = true;
            } else {
                print_usage();
                return 1;
            }
        }
    }

    if (run_scorer) {
        test_factor_normalization();
        test_mode_scorer_all_modes();
        test_mode_scorer_edge_cases();
        test_confidence_and_parameters();
    }
    if (run_hysteresis) {
        test_hysteresis();
        test_hysteresis_stability_30s();
    }
    if (run_engine) test_decision_engine_logging();
    if (run_benchmark) {
        test_benchmark_evaluate();
        test_benchmark_switch_latency();
    }
    if (run_csv) test_csv_validation(csv_path);

    if (failures > 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

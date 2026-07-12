#include "mili/confidence.hpp"
#include "mili/decision_engine.hpp"
#include "mili/mode_scorer.hpp"
#include "mili/mode_state_machine.hpp"
#include "mili/multi_objective.hpp"
#include "mili/sensor_noise_model.hpp"
#include "mili/topsis_scorer.hpp"
#include "mili/weight_calibrator.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

mili::SensorInput make_reconnaissance() {
    mili::SensorInput input{};
    input.battery_level = 95.0f;
    input.threat_level = 0.05f;
    input.comm_strength = 0.95f;
    input.target_visibility = 0.4f;
    input.distance_to_home_km = 2.0f;
    return input;
}

mili::SensorInput make_surveillance() {
    mili::SensorInput input{};
    input.battery_level = 35.0f;
    input.threat_level = 0.05f;
    input.target_visibility = 0.88f;
    input.comm_strength = 0.3f;
    input.distance_to_home_km = 10.0f;
    return input;
}

mili::SensorInput make_pursuit() {
    mili::SensorInput input{};
    input.battery_level = 75.0f;
    input.threat_level = 0.2f;
    input.target_visibility = 0.92f;
    input.comm_strength = 0.8f;
    input.distance_to_home_km = 8.0f;
    input.mission_priority = mili::MissionPriority::High;
    return input;
}

mili::SensorInput make_loiter() {
    mili::SensorInput input{};
    input.comm_strength = 0.1f;
    input.threat_level = 0.85f;
    input.target_visibility = 0.1f;
    input.battery_level = 60.0f;
    input.distance_to_home_km = 15.0f;
    input.mission_priority = mili::MissionPriority::Low;
    return input;
}

mili::SensorInput make_return_home() {
    mili::SensorInput input{};
    input.battery_level = 12.0f;
    input.threat_level = 0.85f;
    input.comm_strength = 0.2f;
    input.distance_to_home_km = 1.0f;
    return input;
}

mili::SensorInput make_engagement() {
    mili::SensorInput input{};
    input.battery_level = 80.0f;
    input.threat_level = 0.9f;
    input.target_visibility = 0.95f;
    input.mission_priority = mili::MissionPriority::Critical;
    input.distance_to_home_km = 4.0f;
    return input;
}

void test_topsis_closeness_range() {
    mili::TopsisScorer scorer;
    const auto result = scorer.analyze(make_engagement());
    for (std::size_t i = 0; i < mili::kModeCount; ++i) {
        check(result.closeness[i] >= 0.0f && result.closeness[i] <= 1.0f, "TOPSIS closeness in [0,1]");
    }
    check(result.closeness[static_cast<std::size_t>(mili::OperationalMode::Engagement)] > 0.3f,
        "Engagement should have meaningful TOPSIS closeness");
}

void test_multi_objective_tradeoff() {
    mili::MultiObjectiveEvaluator evaluator;
    const auto pursuit = evaluator.evaluate_mode(mili::OperationalMode::Pursuit, make_pursuit());
    const auto loiter = evaluator.evaluate_mode(mili::OperationalMode::Loiter, make_loiter());
    check(pursuit.mission_success > loiter.mission_success, "Pursuit should favor mission success");
    check(loiter.energy > pursuit.energy, "Loiter should favor energy");
}

void test_fsm_emergency_transition() {
    mili::ModeStateMachine fsm;
    mili::SensorInput low_battery = make_return_home();
    const auto result = fsm.evaluate_transition(
        mili::OperationalMode::Engagement, low_battery, 0.9f, false);
    check(result.is_emergency, "Low battery should trigger emergency FSM transition");
    check(result.next_state == mili::FsmState::Emergency, "Emergency state should be selected");
}

void test_sensor_noise_model() {
    mili::SensorNoiseModel noise;
    const auto input = make_surveillance();
    bool changed = false;
    for (std::uint32_t seed = 1; seed < 500; ++seed) {
        const auto noisy = noise.apply(input, 0.3f, seed);
        if (std::fabs(noisy.threat_level - input.threat_level) > 0.001f) {
            changed = true;
            break;
        }
    }
    check(changed, "Noise model should perturb inputs under low confidence");
}

void test_mcdm_engine_prefers_expected_mode() {
    mili::McdmEngine engine;
    const std::vector<std::pair<mili::SensorInput, mili::OperationalMode>> cases = {
        {make_reconnaissance(), mili::OperationalMode::Reconnaissance},
        {make_surveillance(), mili::OperationalMode::Surveillance},
        {make_pursuit(), mili::OperationalMode::Pursuit},
        {make_loiter(), mili::OperationalMode::Loiter},
        {make_return_home(), mili::OperationalMode::ReturnHome},
        {make_engagement(), mili::OperationalMode::Engagement},
    };

    for (const auto& test_case : cases) {
        const auto scores = engine.compute_scores(test_case.first);
        const auto best = mili::select_best_mode(scores);
        check(best == test_case.second, mili::to_string(test_case.second));
    }
}

void test_combined_edge_cases() {
    mili::DecisionEngine engine(0);
    engine.set_hysteresis_seconds(0.0f);

    mili::SensorInput rain_night = make_surveillance();
    rain_night.weather = mili::Weather::Rain;
    rain_night.time_of_day = mili::TimeOfDay::Night;
    const auto degraded = engine.evaluate(rain_night, 100);
    check(degraded.confidence > 0.0f, "Rain/night combined scenario should still decide");

    mili::SensorInput critical = make_engagement();
    critical.mission_priority = mili::MissionPriority::Critical;
    const auto critical_result = engine.evaluate(critical, 200);
    check(critical_result.selected_score > 0.0f, "Critical combined scenario should score");
}

void test_weight_calibrator() {
    mili::WeightCalibrator calibrator;
    check(calibrator.load_field_data_csv("data/field_calibration_data.csv"), "Field calibration CSV should load");
    auto config = mili::make_default_weight_config();
    const auto metrics = calibrator.calibrate(config);
    check(metrics.samples > 0, "Calibration should process field samples");
    check(metrics.accuracy_after >= metrics.accuracy_before, "Calibration should not reduce accuracy");
}

}  // namespace

int main() {
    test_topsis_closeness_range();
    test_multi_objective_tradeoff();
    test_fsm_emergency_transition();
    test_sensor_noise_model();
    test_mcdm_engine_prefers_expected_mode();
    test_combined_edge_cases();
    test_weight_calibrator();

    if (failures > 0) {
        std::cerr << failures << " algorithm test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All algorithm tests passed.\n";
    return EXIT_SUCCESS;
}

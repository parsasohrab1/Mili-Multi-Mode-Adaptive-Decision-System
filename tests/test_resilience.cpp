#include "mili/data_quality_monitor.hpp"
#include "mili/decision_engine.hpp"
#include "mili/health_monitor.hpp"
#include "mili/integration/integration_hub.hpp"
#include "mili/integration/control_publisher.hpp"
#include "mili/low_confidence_policy.hpp"
#include "mili/mock_products.hpp"
#include "mili/sensor_imputer.hpp"
#include "mili/thread_safe_engine.hpp"

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_data_quality_missing_stale_invalid() {
    mili::DataQualityMonitor monitor(500);

    mili::Product1Data p1{};
    mili::Product2Data p2{};
    mili::EnvironmentalData env{};
    mili::OperatorCommand op{};

    const auto all_missing = monitor.evaluate(p1, p2, env, op, 1000);
    check(all_missing.missing_count == 3, "All missing sources detected");
    check(all_missing.emergency_return, "Three missing should force emergency");

    p1 = mili::MockProduct1{}.sample(1000, 80.0f);
    p2 = mili::MockProduct2{}.sample(1000, 5.0f);
    env = mili::mock_environmental(1000);
    const auto all_ok = monitor.evaluate(p1, p2, env, op, 1000);
    check(all_ok.product1_quality == mili::SourceQuality::Ok, "Valid product1");
    check(all_ok.degraded_count == 0, "No degradation when all valid");

    p1.battery_level_percent = 150.0f;
    const auto invalid_p1 = monitor.evaluate(p1, p2, env, op, 1000);
    check(invalid_p1.product1_invalid, "Out-of-range battery flagged invalid");
    check(invalid_p1.product1_quality == mili::SourceQuality::Invalid, "Invalid quality enum");

    p1 = mili::MockProduct1{}.sample(1000, 80.0f);
    p1.timestamp_ms = 100;
    const auto stale = monitor.evaluate(p1, p2, env, op, 2000);
    check(stale.product1_stale, "Stale product1 detected");
    check(stale.product1_quality == mili::SourceQuality::Stale, "Stale quality enum");
}

void test_emergency_two_degraded_sources() {
    mili::DataQualityMonitor monitor(500);
    mili::Product1Data p1 = mili::MockProduct1{}.sample(1000, 70.0f);
    mili::Product2Data p2{};
    mili::EnvironmentalData env{};
    mili::OperatorCommand op{};

    const auto status = monitor.evaluate(p1, p2, env, op, 1000);
    check(status.degraded_count == 2, "One present + two missing = 2 degraded");
    check(status.emergency_return, "Two degraded critical sources trigger emergency");
    check(monitor.should_force_return_home(status, 80.0f), "Should force return home");
}

void test_sensor_imputation() {
    mili::DataQualityMonitor monitor(500);
    mili::SensorInput raw{};
    raw.battery_level = 20.0f;
    raw.threat_level = 0.9f;

    mili::Product1Data p1{};
    mili::Product2Data p2 = mili::MockProduct2{}.sample(1000, 4.0f);
    mili::EnvironmentalData env{};
    const auto quality = monitor.evaluate(p1, p2, env, {}, 1000);

    const auto imputed = mili::SensorImputer::apply(raw, quality);
    check(imputed.weights.battery < 1.0f, "Missing battery reduces weight");
    check(imputed.input.battery_level == 50.0f, "Missing battery imputed to safe default");

    std::array<float, mili::kModeCount> scores{};
    scores.fill(1.0f);
    mili::SensorImputer::scale_scores(scores, imputed.weights);
    check(scores[0] < 1.0f, "Imputation scales scores down");
}

void test_low_confidence_policy() {
    check(
        mili::apply_low_confidence_policy(mili::OperationalMode::Engagement, 0.3f)
            == mili::OperationalMode::Reconnaissance,
        "Engagement downgraded under low confidence");

    mili::ModeParameters params{};
    params.sensor_aggressiveness = 0.8f;
    params.engagement_readiness = 0.8f;
    mili::apply_low_confidence_parameters(params, 0.3f);
    check(params.sensor_aggressiveness == 0.4f, "Aggressiveness halved");
    check(params.engagement_readiness == 0.4f, "Engagement readiness halved");

    mili::DecisionEngine engine(0);
    engine.set_noise_enabled(false);
    engine.set_mcdm_enabled(false);

    mili::SensorInput flat{};
    flat.battery_level = 50.0f;
    flat.threat_level = 0.5f;
    flat.comm_strength = 0.5f;
    flat.target_visibility = 0.5f;
    flat.distance_to_home_km = 10.0f;

    const auto result = engine.evaluate(flat, 1000);
    if (result.confidence < mili::kLowConfidenceThreshold) {
        check(result.low_confidence, "Low confidence flag set");
        check(result.selected_mode != mili::OperationalMode::Engagement, "Avoid engagement when uncertain");
    }
}

void test_health_watchdog() {
    mili::SystemHealthMonitor health(100, 300);
    health.on_cycle(0);
    health.on_cycle(100);
    check(health.is_healthy(), "Regular cycles healthy");

    health.on_cycle(800);
    check(health.watchdog_triggered(), "Large gap triggers watchdog");
    check(!health.is_healthy(), "Unhealthy after watchdog");
}

void test_thread_safe_engine() {
    mili::DecisionEngine engine(0);
    engine.set_noise_enabled(false);
    mili::ThreadSafeDecisionEngine safe(engine);

    std::atomic<int> errors{0};
    auto worker = [&]() {
        mili::SensorInput input{};
        input.battery_level = 70.0f;
        for (int i = 0; i < 50; ++i) {
            const auto result = safe.evaluate(input, static_cast<std::uint64_t>(i * 10));
            if (result.confidence <= 0.0f) {
                ++errors;
            }
        }
    };

    std::thread t1(worker);
    std::thread t2(worker);
    t1.join();
    t2.join();
    check(errors.load() == 0, "Concurrent evaluate should not produce invalid results");
}

void test_fuzzing_20_percent_dropout() {
    mili::DecisionEngine engine(0);
    engine.set_noise_enabled(false);

    std::srand(2026);
    int emergencies = 0;
    int low_confidence = 0;
    constexpr int kIterations = 500;

    for (int i = 0; i < kIterations; ++i) {
        mili::DataQualityMonitor monitor(500);
        mili::Product1Data p1 = mili::MockProduct1{}.sample(static_cast<std::uint64_t>(i), 65.0f);
        mili::Product2Data p2 = mili::MockProduct2{}.sample(static_cast<std::uint64_t>(i), 6.0f);
        mili::EnvironmentalData env = mili::mock_environmental(static_cast<std::uint64_t>(i));
        mili::OperatorCommand op{};

        const int roll = std::rand() % 100;
        if (roll < 20) {
            p1.valid = false;
        } else if (roll < 40) {
            p2.valid = false;
        } else if (roll < 60) {
            env.valid = false;
        } else if (roll < 70) {
            p1.battery_level_percent = 200.0f;
        }

        const auto quality = monitor.evaluate(p1, p2, env, op, static_cast<std::uint64_t>(i * 100));
        mili::SensorInput input{};
        input.battery_level = p1.valid ? p1.battery_level_percent : 50.0f;
        input.distance_to_home_km = p2.valid ? p2.distance_to_home_km : 5.0f;
        input.threat_level = env.valid ? env.threat_level : 0.5f;
        input.comm_strength = env.valid ? env.comm_strength : 0.5f;
        input.target_visibility = env.valid ? env.target_visibility : 0.5f;

        const auto imputed = mili::SensorImputer::apply(input, quality);
        mili::EvaluateContext context{};
        context.quality = &quality;
        context.imputation = &imputed.weights;
        context.quality_emergency = quality.emergency_return;

        const auto result = engine.evaluate(imputed.input, static_cast<std::uint64_t>(i * 100), context);
        check(std::isfinite(result.selected_score), "Fuzz scores must be finite");
        if (result.emergency_forced) {
            ++emergencies;
        }
        if (result.low_confidence) {
            ++low_confidence;
        }
    }

    std::cout << "Fuzz: emergencies=" << emergencies << " low_confidence=" << low_confidence << "\n";
    check(emergencies > 0, "Fuzz should trigger some emergency paths");
}

}  // namespace

int main() {
    test_data_quality_missing_stale_invalid();
    test_emergency_two_degraded_sources();
    test_sensor_imputation();
    test_low_confidence_policy();
    test_health_watchdog();
    test_thread_safe_engine();
    test_fuzzing_20_percent_dropout();

    if (failures > 0) {
        std::cerr << failures << " resilience test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All resilience tests passed.\n";
    return EXIT_SUCCESS;
}

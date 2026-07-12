#include "mili/config_version.hpp"
#include "mili/decision_engine.hpp"
#include "mili/mode_scorer.hpp"
#include "mili/types.hpp"
#include "mili/weight_manager.hpp"

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

mili::SensorInput make_engagement_sensitive_input() {
    mili::SensorInput input{};
    input.battery_level = 80.0f;
    input.threat_level = 0.65f;
    input.comm_strength = 0.8f;
    input.target_visibility = 0.9f;
    input.mission_priority = mili::MissionPriority::Critical;
    input.distance_to_home_km = 5.0f;
    return input;
}

void test_load_default_config_file() {
    mili::WeightManager manager;
    const bool loaded = manager.load_from_file("config/default_weights.json");

    check(loaded, "Default config file should load");
    check(!manager.using_defaults(), "Manager should use loaded config after successful load");
    check_near(
        manager.config().engagement.target_visibility,
        0.5f,
        0.001f,
        "Default engagement visibility weight");
}

void test_invalid_config_fallback() {
    mili::WeightManager manager;
    const auto defaults = mili::WeightManager::default_config();

    check(!manager.load_from_file("tests/fixtures/invalid_config.json"), "Invalid config must fail");
    check(manager.using_defaults(), "Invalid config should fallback to defaults");
    check_near(
        manager.config().engagement.target_visibility,
        defaults.engagement.target_visibility,
        0.001f,
        "Fallback engagement weight should match defaults");
}

void test_wrong_version_config_rejected() {
    mili::WeightManager manager;
    check(!manager.load_from_file("tests/fixtures/wrong_version_config.json"), "Wrong major version must fail");
    check(manager.using_defaults(), "Wrong version should fallback to defaults");
    check(mili::is_compatible_weight_config("1.0.0"), "1.0.0 is compatible");
    check(mili::is_compatible_weight_config("1.2.3"), "1.x.x same major is compatible");
    check(!mili::is_compatible_weight_config("2.0.0"), "2.0.0 is incompatible");
}

void test_runtime_engagement_weight_change() {
    mili::DecisionEngine engine;
    const auto input = make_engagement_sensitive_input();

    const auto before = engine.scorer().compute_scores(input);
    const auto engagement_before = before[static_cast<std::size_t>(mili::OperationalMode::Engagement)];

    check(
        engine.set_mode_weight(mili::OperationalMode::Engagement, "target_visibility", 0.8f),
        "Runtime engagement weight update should succeed");

    const auto after = engine.scorer().compute_scores(input);
    const auto engagement_after = after[static_cast<std::size_t>(mili::OperationalMode::Engagement)];

    const float delta = engagement_after - engagement_before;
    std::cout << "Engagement score delta after weight change: " << delta << "\n";

    check(delta > 0.25f, "Engagement weight 0.5 -> 0.8 should materially increase score");
    check_near(
        engine.weight_manager().config().engagement.target_visibility,
        0.8f,
        0.001f,
        "Updated engagement weight should be stored in manager");
}

void test_reload_performance() {
    mili::WeightManager manager;
    check(manager.load_from_file("config/default_weights.json"), "Config must load for reload benchmark");

    constexpr int iterations = 1000;
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        manager.reload();
    }
    const auto end = std::chrono::steady_clock::now();

    const auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double avg_ms = static_cast<double>(total_ms) / iterations;

    std::cout << "Reload benchmark: average = " << avg_ms << " ms\n";
    check(avg_ms < 10.0, "Config reload should average below 10ms");
}

void test_reload_memory_stability() {
    mili::WeightManager manager;
    check(manager.load_from_file("config/default_weights.json"), "Config must load for memory stability test");

    const std::size_t initial_capacity = manager.buffer_capacity();
    constexpr int iterations = 10000;

    for (int i = 0; i < iterations; ++i) {
        manager.reload();
    }

    const std::size_t final_capacity = manager.buffer_capacity();
    std::cout << "Buffer capacity initial=" << initial_capacity << " final=" << final_capacity << "\n";

    check(manager.reload_count() >= static_cast<std::size_t>(iterations), "Reload count should track reloads");
    check(
        final_capacity <= initial_capacity + 64,
        "Reload buffer capacity should remain stable across 10000 reloads");
    check_near(
        manager.config().engagement.target_visibility,
        0.5f,
        0.001f,
        "Weights should remain valid after repeated reload");
}

void test_dependency_injection() {
    mili::WeightManager external_weights;
    mili::ModeScorer external_scorer(&external_weights);
    mili::HysteresisController external_hysteresis(3.0f);
    mili::DecisionLogger external_logger(128);

    mili::EngineComponents components{};
    components.weight_manager = &external_weights;
    components.scorer = &external_scorer;
    components.hysteresis = &external_hysteresis;
    components.logger = &external_logger;

    mili::DecisionEngine engine(components, 0);
    mili::SensorInput input{};
    input.battery_level = 90.0f;
    input.threat_level = 0.1f;

    engine.evaluate(input, 0);

    check(engine.logger().size() == 1, "Injected logger should receive decision entries");
    check_near(engine.hysteresis_seconds(), 3.0f, 0.001f, "Injected hysteresis should be used");

    external_weights.set_mode_factor(mili::OperationalMode::Engagement, "target_visibility", 0.8f);
    check_near(
        engine.scorer().compute_scores(input)[static_cast<std::size_t>(mili::OperationalMode::Engagement)],
        external_scorer.compute_scores(input)[static_cast<std::size_t>(mili::OperationalMode::Engagement)],
        0.001f,
        "Injected scorer and engine should share weight manager state");
}

}  // namespace

int main() {
    test_load_default_config_file();
    test_invalid_config_fallback();
    test_wrong_version_config_rejected();
    test_runtime_engagement_weight_change();
    test_reload_performance();
    test_reload_memory_stability();
    test_dependency_injection();

    if (failures > 0) {
        std::cerr << failures << " weight manager test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All weight manager tests passed.\n";
    return EXIT_SUCCESS;
}

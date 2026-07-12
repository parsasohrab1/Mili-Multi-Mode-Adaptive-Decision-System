cmake_minimum_required(VERSION 3.16)

project(MiliDecisionSystem VERSION 1.0.0 LANGUAGES CXX)



set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



option(MILI_BUILD_TESTS "Build unit tests" ON)

option(MILI_BUILD_SIMULATOR "Build host simulator executable" ON)

option(MILI_BUILD_EMBEDDED_HAL "Build STM32 HAL stubs and embedded app module" ON)

option(MILI_EMBEDDED "Cross-compile for STM32H7 (requires arm-gcc toolchain)" OFF)

option(MILI_ENABLE_COVERAGE "Enable gcov/lcov code coverage (GCC/Clang only)" OFF)



set(MILI_CORE_SOURCES

    src/types.cpp

    src/factor_normalizer.cpp

    src/confidence.cpp

    src/weight_config.cpp

    src/weight_manager.cpp

    src/mode_parameters.cpp

    src/mode_scorer.cpp

    src/hysteresis_controller.cpp

    src/decision_engine.cpp

    src/product1_adapter.cpp

    src/product2_adapter.cpp

    src/operator_command_handler.cpp

    src/sensor_adapter.cpp

    src/decision_loop.cpp

    src/mock_products.cpp

    src/protocol/messages.cpp

    src/integration/integration_hub.cpp

    src/integration/timestamp_sync.cpp

    src/integration/product1_api.cpp

    src/integration/product2_api.cpp

    src/integration/control_publisher.cpp

    src/schema/data_contract.cpp

    src/hil/hil_runner.cpp

    src/hil/soak_runner.cpp

    src/data_quality_monitor.cpp

    src/sensor_imputer.cpp

    src/low_confidence_policy.cpp

    src/health_monitor.cpp

    src/thread_safe_engine.cpp

    src/topsis_scorer.cpp

    src/multi_objective.cpp

    src/mode_state_machine.cpp

    src/sensor_noise_model.cpp

    src/platform/can_router.cpp

    src/platform/benchmark.cpp

    src/config_version.cpp

    src/log_format.cpp

    src/uart_log_exporter.cpp

)



if(MILI_EMBEDDED)

    list(APPEND MILI_CORE_SOURCES

        platform/stm32/hal/can_hal.cpp

        platform/stm32/hal/uart_hal.cpp

        platform/stm32/hal/spi_hal.cpp

        platform/stm32/hal/cycle_counter.cpp

    )

else()

    list(APPEND MILI_CORE_SOURCES

        src/decision_logger.cpp

        src/log_exporter.cpp

        src/log_filter.cpp

        src/weight_calibrator.cpp

    )

    if(MILI_BUILD_EMBEDDED_HAL)

        list(APPEND MILI_CORE_SOURCES

            platform/stm32/hal/can_hal.cpp

            platform/stm32/hal/uart_hal.cpp

            platform/stm32/hal/spi_hal.cpp

            platform/stm32/hal/cycle_counter.cpp

            platform/stm32/App/mili_app.cpp

        )

    endif()

endif()



add_library(mili_decision STATIC ${MILI_CORE_SOURCES})



target_include_directories(mili_decision

    PUBLIC

        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platform/stm32>

        $<INSTALL_INTERFACE:include>

)



if(MILI_EMBEDDED)

    target_compile_definitions(mili_decision PUBLIC MILI_EMBEDDED MILI_STM32_DWT)

endif()



if(MILI_ENABLE_COVERAGE AND NOT MILI_EMBEDDED)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

        add_compile_options(--coverage -O0 -g)

        add_link_options(--coverage)

    endif()

endif()



target_compile_options(mili_decision PRIVATE

    $<$<CXX_COMPILER_ID:MSVC>:/W4>

    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>

)



if(MILI_BUILD_SIMULATOR AND NOT MILI_EMBEDDED)

    add_executable(mili_simulator src/main.cpp)

    target_link_libraries(mili_simulator PRIVATE mili_decision)

endif()



if(MILI_BUILD_TESTS AND NOT MILI_EMBEDDED)

    enable_testing()

    add_subdirectory(tests)

endif()



if(MILI_EMBEDDED)

    add_subdirectory(platform/stm32)

endif()



install(TARGETS mili_decision

    ARCHIVE DESTINATION lib

    LIBRARY DESTINATION lib

)



install(DIRECTORY include/ DESTINATION include)


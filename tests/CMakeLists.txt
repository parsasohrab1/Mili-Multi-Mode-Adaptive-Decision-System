cmake_minimum_required(VERSION 3.16)

add_executable(mili_tests
    test_decision_engine.cpp
)

add_executable(mili_weight_tests
    test_weight_manager.cpp
)

add_executable(mili_integration_tests
    test_integration.cpp
)

add_executable(mili_evolution_tests
    test_evolution.cpp
)

add_executable(mili_algorithm_tests
    test_algorithm.cpp
)

add_executable(mili_embedded_tests
    test_embedded.cpp
)

add_executable(mili_hil_tests
    test_hil.cpp
)

target_include_directories(mili_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mili_weight_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mili_integration_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mili_evolution_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mili_algorithm_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mili_embedded_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/platform/stm32/App
    ${CMAKE_SOURCE_DIR}/platform/stm32/hal
)
add_executable(mili_resilience_tests
    test_resilience.cpp
)

target_include_directories(mili_hil_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(mili_reporting_tests
    test_reporting.cpp
)

target_include_directories(mili_resilience_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(mili_field_qa_tests
    test_field_qa.cpp
)

target_include_directories(mili_reporting_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mili_field_qa_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/platform/stm32/App
    ${CMAKE_SOURCE_DIR}/platform/stm32/hal
)
target_link_libraries(mili_tests PRIVATE mili_decision)
target_link_libraries(mili_weight_tests PRIVATE mili_decision)
target_link_libraries(mili_integration_tests PRIVATE mili_decision)
target_link_libraries(mili_evolution_tests PRIVATE mili_decision)
target_link_libraries(mili_algorithm_tests PRIVATE mili_decision)
target_link_libraries(mili_embedded_tests PRIVATE mili_decision)
target_link_libraries(mili_hil_tests PRIVATE mili_decision)
target_link_libraries(mili_resilience_tests PRIVATE mili_decision)
target_link_libraries(mili_reporting_tests PRIVATE mili_decision)
target_link_libraries(mili_field_qa_tests PRIVATE mili_decision)

add_test(NAME ModeScorerTests COMMAND mili_tests --mode-scorer)
add_test(NAME HysteresisTests COMMAND mili_tests --hysteresis)
add_test(NAME DecisionEngineTests COMMAND mili_tests --decision-engine)
add_test(NAME BenchmarkTests COMMAND mili_tests --benchmark)
add_test(NAME CsvValidationTests COMMAND mili_tests --csv-validation ${CMAKE_SOURCE_DIR}/data/mission_decision_data.csv)
add_test(NAME WeightManagerTests COMMAND mili_weight_tests)
add_test(NAME IntegrationTests COMMAND mili_integration_tests)
add_test(NAME EvolutionTests COMMAND mili_evolution_tests)
add_test(NAME AlgorithmTests COMMAND mili_algorithm_tests)
add_test(NAME EmbeddedTests COMMAND mili_embedded_tests)
add_test(NAME HilTests COMMAND mili_hil_tests)
add_test(NAME ResilienceTests COMMAND mili_resilience_tests)
add_test(NAME ReportingTests COMMAND mili_reporting_tests)
add_test(NAME FieldQaTests COMMAND mili_field_qa_tests)

set(TEST_WORKING_DIR ${CMAKE_SOURCE_DIR})
foreach(test_name IN ITEMS ModeScorerTests HysteresisTests DecisionEngineTests BenchmarkTests CsvValidationTests WeightManagerTests IntegrationTests EvolutionTests AlgorithmTests EmbeddedTests HilTests ResilienceTests ReportingTests FieldQaTests)
    set_tests_properties(${test_name} PROPERTIES WORKING_DIRECTORY ${TEST_WORKING_DIR})
endforeach()

# Embedded firmware target (requires arm-none-eabi-gcc + STM32Cube HAL linkage).

add_executable(mili_stm32_firmware
    App/mili_app.cpp
    hal/can_hal.cpp
    hal/uart_hal.cpp
    hal/spi_hal.cpp
    hal/cycle_counter.cpp
)

target_include_directories(mili_stm32_firmware PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/App
    ${CMAKE_CURRENT_SOURCE_DIR}/hal
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(mili_stm32_firmware PRIVATE mili_decision)

target_compile_definitions(mili_stm32_firmware PRIVATE
    MILI_EMBEDDED
    MILI_STM32_DWT
)

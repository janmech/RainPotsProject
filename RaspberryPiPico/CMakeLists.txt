# What CMake to start at
cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
 
# Include the subsidiary .cmake file to get the SDK
include(pico_sdk_import.cmake)
 
# Set the name and version of the project
project(RainPots C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()


list(APPEND list_executables source_files)
include(include_project_files.cmake)



# Link the Project to a source file (step 4.6)
add_executable(RainPots RainPots.cpp ${list_executables})

pico_generate_pio_header(RainPots ${CMAKE_CURRENT_LIST_DIR}/shift_in_out.pio)


target_sources(RainPots PRIVATE RainPots.cpp)

target_include_directories(
    RainPots
    PUBLIC
    ${header_dir_list} # Recursive
)

pico_add_extra_outputs(RainPots)

 
# Link the Project to an extra library (pico_stdlib)ç
target_link_libraries(RainPots PRIVATE
    pico_stdlib
    pico_multicore
    hardware_uart 
    hardware_gpio 
    hardware_i2c
    hardware_flash 
    hardware_adc 
    hardware_irq
    hardware_pio
    hardware_pwm
    )
 
 
# Enable USB, UART output
# stdio must be disabled. We use uart0 for MIDI In/Out
pico_enable_stdio_usb(RainPots 1)  
pico_enable_stdio_uart(RainPots 0)
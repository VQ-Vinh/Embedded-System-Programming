cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Src)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
# * Mã ứng dụng cân kiểm tra: driver (App/Drivers), logic thuần (App/Services)
#   và 5 task FreeRTOS (App/Tasks). Thêm file mới vào đúng nhóm bên dưới.
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    App/Drivers/dwt_delay.c
    App/Drivers/hx711.c
    App/Drivers/keypad.c
    App/Drivers/photo_sensor.c
    App/Drivers/indicator.c
    App/Drivers/flash_store.c
    App/Drivers/tft.c
    App/Services/fmt.c
    App/Services/scale_filter.c
    App/Services/weigh_fsm.c
    App/Services/num_input.c
    App/Services/config.c
    App/Services/app_logic.c
    App/Tasks/app_main.c
    App/Tasks/task_sensor.c
    App/Tasks/task_keypad.c
    App/Tasks/task_main.c
    App/Tasks/task_display.c
    App/Tasks/task_uart.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    App/Drivers
    App/Services
    App/Tasks
)

# ! main.h (do CubeMX quản lý) include app_types.h nên mọi target dùng
# ! stm32cubemx đều cần thấy App/Inc.
target_include_directories(stm32cubemx INTERFACE ${CMAKE_SOURCE_DIR}/App/Inc)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# * Generate programmer-friendly firmware images after the ELF is linked.
# ! CMAKE_OBJCOPY is provided by the selected ARM GNU toolchain file.
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
        $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary
        $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating ${CMAKE_PROJECT_NAME}.hex and ${CMAKE_PROJECT_NAME}.bin"
    VERBATIM
)

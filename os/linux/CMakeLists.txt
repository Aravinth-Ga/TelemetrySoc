# Add telemetry_os_linux library
add_library(telemetry_os_linux
    thread.cpp
    timer.cpp
)

# Include directories
target_include_directories(telemetry_os_linux
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

# Compiler Warnings configuration
add_compile_options(telemetry_os_linux
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)


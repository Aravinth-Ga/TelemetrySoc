# Add telemetry_core library
add_library(telemetry_core
    ring_buffer.c
    memory_pool.c
    event.c
)

# Include directories
target_include_directories(telemetry_core 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler Warnings configuration
target_compile_options(telemetry_core 
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
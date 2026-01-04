# Add telemetry_core library
add_library(telemetry_agent
    telemetry_agent.c

)

# Include directories
target_include_directories(telemetry_agent 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler Warnings configuration
target_compile_options(telemetry_agent 
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
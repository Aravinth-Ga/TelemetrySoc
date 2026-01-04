# Add telemetry_core library
add_library(telemetry_agent
    telemetry_agent.c

)

# Include directories
target_include_directories(telemetry_agent 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link to dependencies
target_link_libraries(telemetry_agent
    PUBLIC
        telemetry_core
        telemetry_os_inlcude
)

# Compiler Warnings configuration
target_compile_options(telemetry_agent 
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
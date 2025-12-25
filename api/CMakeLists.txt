# Add telemetry_api library
add_library(telemetry_api
    telemetry.cpp
)

# Include directories
target_include_directories(telemetry_api
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

# Compiler Warnings configuration
add_compile_options(telemetry_api
    PRIVATE
        -Wall
        -Wextra
        -Wpendantic
)


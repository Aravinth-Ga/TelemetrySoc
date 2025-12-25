# Add telemetry_transport library
add_library(telemetry_transport INTERFACE)

# Include directories
target_include_directories(telemetry_transport
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
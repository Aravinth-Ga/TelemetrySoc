# Add telemetry_transport library
add_library(telemetry_transport STATIC transport_adapter.cpp udp_transport.cpp)

# Include directories
target_include_directories(telemetry_transport
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
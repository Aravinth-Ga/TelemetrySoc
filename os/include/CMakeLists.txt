# Add telemetry_os_inlcude library
add_library(telemetry_os_inlcude INTERFACE)

# Include directories
target_include_directories(telemetry_os_inlcude
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
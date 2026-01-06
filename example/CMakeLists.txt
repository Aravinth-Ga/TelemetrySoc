# Add telemetry_example executable
add_executable(telemetry_example demo.cpp)

# Link telemetry_framework library
target_link_libraries(telemetry_example 
    PRIVATE
        telemetry_os_linux
        telemetry_framework
)
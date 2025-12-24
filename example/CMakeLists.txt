add_executable(telemetry_example demo.cpp)

target_link_libraries(telemetry_example 
    PRIVATE
        telemetry_framework
)
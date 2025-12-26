add_executable(test_telemetry_framework
    test_event.c
    test_ring_buffer.c
    test_suite.c
)

target_include_directories(test_telemetry_framework 
    PRIVATE
        ${CMAKE_SOURCE_DIR}/core
)

target_link_libraries(test_telemetry_framework 
    PRIVATE 
        telemetry_core
)
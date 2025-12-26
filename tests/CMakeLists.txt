add_executable(test_event
    test_event.c
    test_ring_buffer.c
)

target_include_directories(test_event 
    PRIVATE
        ${CMAKE_SOURCE_DIR}/core
)

target_link_libraries(test_event 
    PRIVATE 
        telemetry_core
)
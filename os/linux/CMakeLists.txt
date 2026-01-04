# Add telemetry_os_linux library
add_library(telemetry_os_linux
    osal_thread_linux.c
    osal_time_linux.c
    osal_wakeup_linux.c
)

# Include directories
target_include_directories(telemetry_os_linux 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link to OS abstraction layer includes
target_link_libraries(telemetry_os_linux
    PUBLIC
        telemetry_os_inlcude
)

# Compiler Warnings configuration
target_compile_options(telemetry_os_linux 
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)


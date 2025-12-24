add_library(telemetry_os_linux
    thread.cpp
    timer.cpp
)

target_include_directories(telemetry_os_linux
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

add_compile_options(telemetry_os_linux
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)


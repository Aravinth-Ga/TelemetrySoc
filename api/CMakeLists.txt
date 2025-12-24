add_library(telemetry_api
    telemetry.cpp
)

target_include_directories(telemetry_api
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

add_compile_options(telemetry_api
    PRIVATE
        -Wall
        -Wextra
        -Wpendantic
)


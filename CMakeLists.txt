# 1. Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# 2. Project, Version and Language
project(TELEMETRY4SOC
    VERSION 1.0.0
    DESCRIPTION "Telemetry for System on Chip"
    LANGUAGES C CXX)

# 3. Set the Standard for C and C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXTENSIONS OFF)

Set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TELEMETRY_BUILD_EXAMPLES "Build the example applications" ON)
option(TELEMETRY_BUILD_TESTS "Build the test suites" ON)

# 4. Add the libraries/modules
add_subdirectory(core)
add_subdirectory(api)
add_subdirectory(os/linux)
add_subdirectory(transport)

# 5. Create the main telemetry framework interface library
add_library(telemetry_framework INTERFACE)
target_link_libraries(telemetry_framework INTERFACE
    telemetry_core
    telemetry_api
    telemetry_os_linux
    telemetry_os_inlcude
    telemetry_transport)

if(TELEMETRY_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(TELEMETRY_BUILD_TESTS)
    add_subdirectory(tests)
endif()




cmake_minimum_required(VERSION 3.16)


project(
    ECUDashboard
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    ECUDashboard

    src/main.cpp
    src/dashboard.cpp
    src/ecu.cpp
    src/sensor.cpp
    src/vehicleSimulator.cpp
)


target_include_directories(
    ECUDashboard

    PRIVATE

    $(PROJECT_SOURCE_DIR)/include
)

if(MSVC)

    target_compile_options(
        ECUDashboard
        PRIVATE
        /W4
    )

else()

    target_compile_options(
        ECUDashboard
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()
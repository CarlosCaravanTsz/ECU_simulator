cmake_minimum_required(VERSION 3.16)


project(
    ECUDashboard
    VERSION 1.0
    DESCRIPTION "Simulador de ECU automotriz con dashboard"
    LANGUAGES CXX
)

# Configuración de C++

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Biblioteca principal del proyecto

add_library(
    ecu_core

    src/dashboard.cpp
    src/ecu.cpp
    src/sensor.cpp
    src/vehicleSimulator.cpp
)

# Headers

target_include_directories(
    ecu_core

    PUBLIC

    $(PROJECT_SOURCE_DIR)/include
)


# Advertencias del compilador


if(MSVC)

    target_compile_options(
        ecu_core
        PRIVATE
        /W4
    )

else()

    target_compile_options(
        ecu_core
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()


# Ejecutable


add_executable(
    ECUDashboard

    src/main.cpp
)


# Enlazar ejecutable con la biblioteca

target_link_libraries(
    ECUDashboard

    PRIVATE

    ecu_core
)

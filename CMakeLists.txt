# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14) # Updated for FetchContent

# Define the project name and C++ version
project(TrafficSimulator VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build raylib examples" OFF)
set(CUSTOMIZE_BUILD ON)
set(PLATFORM "Desktop") # Force Desktop platform for Windows

add_subdirectory(libs/raylib)

# Find dependencies for libosmium
find_package(ZLIB REQUIRED)
find_package(EXPAT REQUIRED)
find_package(BZip2 REQUIRED)

# --- Core Library ---
add_library(TrafficSimulatorLib STATIC
    src/Benchmarker.cpp
    src/Engine.cpp
    src/Vehicle.cpp
    src/Map.cpp
    src/Quadtree.cpp
    src/Pathfinder.cpp
    src/ThreadPool.cpp
    src/Simulation.cpp
    src/Renderer.cpp
    src/InputController.cpp
    src/Gui.cpp
)

target_include_directories(TrafficSimulatorLib PUBLIC
    include
    libs/libosmium/include
    libs/protozero/include
    ${BZIP2_INCLUDE_DIRS}
)

target_link_libraries(TrafficSimulatorLib PUBLIC
    raylib
    ${ZLIB_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${BZIP2_LIBRARIES}
)

if(WIN32)
    target_compile_definitions(TrafficSimulatorLib PUBLIC
        _USE_MATH_DEFINES
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
        NOMINMAX
    )
    target_link_libraries(TrafficSimulatorLib PUBLIC winmm gdi32 shell32)
endif()

# --- Main Executable ---
add_executable(TrafficSimulator
    src/main.cpp
)

target_link_libraries(TrafficSimulator PRIVATE TrafficSimulatorLib)

# --- Testing ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
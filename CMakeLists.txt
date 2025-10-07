# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

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

# Create executable
add_executable(TrafficSimulator
    src/main.cpp
    src/Engine.cpp
    src/Vehicle.cpp
    src/Map.cpp
)

# Tell our executable where to find its header files
target_include_directories(TrafficSimulator PRIVATE
    include
    libs/libosmium/include
    libs/protozero/include
    ${BZIP2_INCLUDE_DIRS}
)

# Link our game executable against its libraries
target_link_libraries(TrafficSimulator PRIVATE
    raylib
    ${ZLIB_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${BZIP2_LIBRARIES}
)

# Add definitions and libraries needed for Windows
if(WIN32)
    target_compile_definitions(TrafficSimulator PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
        NOMINMAX
    )
    target_link_libraries(TrafficSimulator PRIVATE winmm gdi32 shell32)
endif()
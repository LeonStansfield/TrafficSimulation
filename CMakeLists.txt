# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Define the project name and C++ version
project(TrafficSimulator VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to build the raylib library from the subdirectory
add_subdirectory(raylib)

# Create our game executable from all our source files
add_executable(TrafficSimulator
    src/main.cpp
    src/Engine.cpp
    src/Ball.cpp
)

# Tell our executable where to find its header files (the 'include' directory)
target_include_directories(TrafficSimulator PRIVATE include)

# Link our game executable against the raylib library
target_link_libraries(TrafficSimulator PRIVATE raylib)
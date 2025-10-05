# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Define the project name and C++ version
project(TrafficSimulator VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to build the raylib library from the subdirectory
# This makes the raylib targets available to our project
add_subdirectory(raylib)

# Create our game executable from our main.cpp file
add_executable(TrafficSimulator src/main.cpp)

# Link our game executable against the raylib library
# This ensures our code can find and use raylib's functions
target_link_libraries(TrafficSimulator PRIVATE raylib)
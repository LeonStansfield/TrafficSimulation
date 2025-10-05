# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Define the project name and C++ version
project(TrafficSimulator VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectory for raylib
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
)

# Link our game executable against its libraries
target_link_libraries(TrafficSimulator PRIVATE
    raylib
    ${ZLIB_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${BZIP2_LIBRARIES}
)
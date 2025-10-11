# Traffic Simulation

A traffic simulation engine using C++ and raylib for my final year university project.

## Functionality

* **`Engine`**:  initializes the window and the main simulation loop. It handles user input (panning and zooming), updates all simulation objects, and draws everything to the screen on each frame.

* **`Map`**: The `Map` class loads and parses `.osm` files. It uses the libosmium library to read OSM data. It identifies all roads and the intersections that connect them, converting the latitude and longitude coordinates into a 2D world space measured in meters. This creates and stores a structured graph of the road network.

* **`Intersection` and `Road`**:
    * An `Intersection` is a point on the map where at least two roads meet.
    * A `Road` is a path that connects two intersections. Each road consists of a series of points that define its shape.

## Setup & Compilation

Clone the required libraries into the libs directory of the project.
Bash

### Navigate to the libs directory
cd libs

### Clone the required repositories
`git clone https://github.com/raysan5/raylib.git`
`git clone https://github.com/osmcode/libosmium.git`
`git clone https://github.com/mapbox/protozero.git`

### Navigate back to the project root
cd ..

On linux:

Configure with CMake: Create a build directory and run CMake from it.

`mkdir build`
`cd build`
`cmake ..`

Compile the Project:
`cmake --build .`

On windows:

Ensure vcpkg is installed:
Open a new terminal and run
`git clone https://github.com/microsoft/vcpkg.git`
`cd vcpkg`

Run the bootstrap script to build vcpkg.
`.\bootstrap-vcpkg.bat`

Install Dependencies with vcpkg:
`.\vcpkg install zlib:x64-windows expat:x64-windows bzip2:x64-windows`

Configure with CMake:
`mkdir build`
`cd build`
`cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake`
(Replace C:\path\to\vcpkg with the actual path to your vcpkg installation.)

Compile the Project:
`cmake --build .`

## Running the Simulation

After a successful compilation, the executable will be located in the build directory.

Linux: 
`./TrafficSimulator PathToOSMFile`

Windows: 
`.\Debug\TrafficSimulator.exe PathToOSMFile` or `.\Release\TrafficSimulator.exe PathToOSMFile`

## Todo
- Step one: Get vehicles spawning randomly on roads
- Step two: Implement basic vehicle movement along roads
- Step three: Implement more advanced vehicle behaviours (stopping and waiting at junctions and braking to stop behind other vehicles)
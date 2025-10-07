# Traffic Simulation

A traffic simulation engine using C++ and raylib for my final year university project.

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
`./TrafficSimulator`

Windows: 
`.\Debug\TrafficSimulator.exe` or `.\Release\TrafficSimulator.exe`
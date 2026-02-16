## Setup & Compilation

Clone the required libraries into the libs directory of the project.

### Navigate to the libs directory
`cd libs`

### Clone the required repositories
`git clone https://github.com/raysan5/raylib.git`
`git clone https://github.com/osmcode/libosmium.git`
`git clone https://github.com/mapbox/protozero.git`

### Navigate back to the project root

`cd ..`

Configure with CMake: Create a build directory and run CMake from it.

`mkdir build`
`cd build`
`cmake ..`

Compile the Project:
`cmake --build .`

### Running Unit Tests

To verify the integrity of the core components (Quadtree, Pathfinder, Map, Vehicle), run the integrated test suite:

1.  Navigate to the build directory:
    `cd build`
2.  Run the tests using CTest:
    `ctest --output-on-failure`

This will execute all registered tests and report any failures.

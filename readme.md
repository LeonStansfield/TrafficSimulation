# Traffic Simulation

A traffic simulation engine using C++ and raylib for my final year university project.

## Functionality

 * **`Engine`**: Initializes the window and the main simulation loop. It handles user input (panning and zooming), updates all simulation objects, and draws everything to the screen on each frame.

    * **`Map`**: The Map class loads and parses .osm files using the libosmium library. It converts latitude and longitude coordinates into a 2D world space measured in meters.

        * **`Directed Graph`**: The map parses OSM tags to build a directed graph of the road network.

        * **`Lane Generation`**: Roads marked as two-way are split into two separate, offset Road objects (one for each direction), creating realistic lanes. One-way roads are represented as a single directed Road.

        * **`Rendering`**: The map has two rendering modes. The normal view draws centered road lines with directional arrows (single for one-way, double for two-way). A debug view toggles drawing the actual offset paths that vehicles follow.

    * **`Intersection` and `Road`**:

        * An Intersection is a node in the graph, a point on the map where at least two roads meet.

        * A Road is a directed edge in the graph. It stores a series of points representing the precise, offset path from a fromIntersectionId to a toIntersectionId.

    * **`Pathfinder`**: A class that encapsulates the pathfinding logic. It uses the A* search algorithm on the Map's directed graph to find the shortest path (by road length) between any two intersections.

    * **`Quadtree`**: A Quadtree is used to spatially partition all vehicles in the simulation. This allows for highly efficient "look-ahead" queries for collision avoidance and junction checks. Instead of checking against every other vehicle, a vehicle only queries against those in its immediate vicinity.

    * **`Vehicle`**: Each Vehicle is an autonomous agent with randomized kinematic properties (max speed, acceleration, etc.) and a state machine (DRIVING, BRAKING, WAITING_JUNCTION).

        * **`Pathfinding`**: When a vehicle is created, it snaps to the nearest road. It requests a path from the Pathfinder class to a randomly selected destination intersection on the map. The vehicle receives a list of roads and follows them in sequence. Upon reaching its destination, it requests a new random path and repeats.

        * **`Curvature Detection`**: Vehicles look ahead on their path and automatically slow down to a safe speed for sharp turns.

        * **`Collision Avoidance`**: Using the Quadtree to limit the number of vehicles it will search through, a vehicle looks ahead for other vehicles on its path near its position. It calculates a safe following distance (based on speed and a time gap) and will brake to avoid a rear-end collision. This logic correctly ignores oncoming traffic, which is on a separate, parallel Road.

        * **`Junction Logic`**: Vehicles detect when they are approaching an intersection. They query the Quadtree for other vehicles in the junction area. If the junction is busy with another vehicle, the vehicle enters a WAITING_JUNCTION state and stops until the path is clear.

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
`./TrafficSimulator PathToOSMFile(str) NumberOfVehicles(int)`

Windows: 
`.\Debug\TrafficSimulator.exe PathToOSMFile(str) NumberOfVehicles(int)` or `.\Release\TrafficSimulator.exe PathToOSMFile(str) NumberOfVehicles(int)`

## Todo
Features:
 - Implement proper vehicle pathfinding to a destination (instead of random outgoing road)
 - Implement heatmap rendering of traffic speed
 - Implement interactivity
    - Click on road to view its stats (number of vehicles travelled on this road, average speed, etc)
    - Click on vehicle to see its planned path
    - Click on vehicle to see its stats (speed, direction, distance travelled, etc)
 - Vehicle speed should be dependent on road type (read from OSM data?)
 - Implement fast simulation mode (runs simulation as fast as possible, rather than in real-time)
    - Config file parsing?

Bugs:

optimisations and improvements:
 - Parallelize/optimise vehicle behavior
 - Parallelize Quadtree
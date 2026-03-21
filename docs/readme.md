# Traffic Simulation

A traffic simulation engine using C++ and raylib for my final year university project.

## Functionality

* **`Engine`**: The Engine class is the main entry point for the application. It initializes the window and runs the main loop. It manages the following core components:

* **`Simulation`**: This class contains the entire state of the simulation, including the map, vehicles, quadtree, and pathfinder. It handles the update logic for all objects and time management.

* **`Renderer`**: This class handles all graphical output. It manages the camera, font loading, and the separate drawing modes. It draws the simulation state to the screen.

* **`Gui`**: This class handles the 2D user interface overlay using RayGui. It displays simulation statistics and specific information for selected objects (vehicles, roads, intersections).

* **`InputController`**: This class processes user input (mouse clicks, keyboard) to update the camera position and handle object selection. It stores the state of the currently selected object.

* **`Multithreading`**: The simulation employs a task-based parallelism approach to efficiently update large numbers of vehicles.
    * **Thread Pool**: A custom `ThreadPool` is initialized with a number of worker threads equal to the hardware concurrency (or a default of 4). These threads remain active throughout the application's lifecycle, waiting for tasks on a condition variable.
    * **Parallel Update Loop**: In each simulation tick, the `Simulation::update` method determines if the number of objects is sufficient to warrant parallel processing (threshold > 100).
    * **Work Partitioning**: The vector of vehicles is logically partitioned into contiguous chunks. The size of each chunk is calculated dynamically based on the total vehicle count and the number of available threads to ensure load balancing.
    * **Task Submission**: A lambda function is created for each chunk, capturing the `this` pointer and the specific start/end indices for that chunk. These lambdas are enqueued into the `ThreadPool`.
    * **Synchronization**: Each enqueue operation returns a `std::future<void>`. The main thread stores these futures in a vector and calls `.get()` on each one. This acts as a barrier synchronization point, ensuring all worker threads have completed their assigned vehicle updates before the main simulation loop proceeds to the rendering phase. This guarantees that the game state is consistent before it is drawn.

* **`Rendering Modes`**: The application supports three rendering modes:
    * **Normal**: Displays roads with standard markings and direction arrows.
    * **Heatmap**: Colors roads based on the average speed of traffic. Green indicates high speed, while red indicates congestion.
    * **Debug**: visualizes the internal logic, showing the exact paths vehicles follow, the quadtree spatial partitioning grid, and connection nodes.

* **`Map`**: The Map class loads and parses .osm files using the libosmium library. It converts latitude and longitude coordinates into a 2D world space measured in meters.

    * **`Directed Graph`**: The map parses OSM tags to build a directed graph of the road network.

    * **`Speed Limits`**: The map parses `maxspeed` tags or infers speed limits based on `highway` types (e.g., motorway, residential) to assign appropriate speed limits to roads.

    * **`Lane Generation`**: Roads marked as two-way are split into two separate, offset Road objects (one for each direction), creating realistic lanes. One-way roads are represented as a single directed Road.

    * **`Intersection` and `Road`**:

         * An Intersection is a node in the graph, a point on the map where at least two roads meet.

        * A Road is a directed edge in the graph. It stores a series of points representing the precise, offset path from a fromIntersectionId to a toIntersectionId.

* **`Pathfinder`**: A class that encapsulates the pathfinding logic. It uses the A* search algorithm on the Map's directed graph to find the shortest path (weighted by time taken to travel the road (road length / speed limit)) between any two intersections.

* **`Quadtree`**: A Quadtree is used to spatially partition all vehicles in the simulation. This allows for efficient spatial queries throughout the simulation.

* **`Vehicle`**: Each Vehicle is an autonomous agent with randomized kinematic properties (max speed, acceleration, etc.) and a state machine (DRIVING, BRAKING, WAITING_JUNCTION). Vehicles respect the speed limits of the roads they are on, modulated by a randomized "speed factor" for realistic variation.

    * **`Pathfinding`**: When a vehicle is created, it snaps to the nearest road. It requests a path from the Pathfinder class to a randomly selected destination intersection on the map. The vehicle receives a list of roads and follows them in sequence. Upon reaching its destination, it requests a new random path and repeats.

    * **`Curvature Detection`**: Vehicles look ahead on their path and automatically slow down to a safe speed for sharp turns.

    * **`Collision Avoidance`**: Using the Quadtree to limit the number of vehicles it will search through, a vehicle looks ahead for other vehicles on its path near its position. It calculates a safe following distance (based on speed and a time gap) and will brake to avoid a rear-end collision. This logic correctly ignores oncoming traffic, which is on a separate, parallel Road.

    * **`Junction Logic`**: Vehicles detect when they are approaching an intersection. They query the Quadtree for other vehicles in the junction area. If the junction is busy with another vehicle, the vehicle enters a WAITING_JUNCTION state and stops until the path is clear.

    * **`Roundabout Logic`**: Vehicles detect when they are approaching a roundabout and behave as if they are at a junction, but with the added rule that they must give way to vehicles already in the roundabout.

    * **`Lane Changing Logic`**: Vehicles can change lanes to overtake slower traffic or to prepare for upcoming turns. They evaluate the traffic in adjacent lanes and switch if a safe opportunity arises, maintaining a safe following distance from vehicles in the target lane. Lane changing is managed using a int to record what lane each vehicle is in, rather than physically moving the vehicle to a new lane.

* **`Interactivity`**: The simulation allows users to interact with the environment using the mouse.

    * **`Selection System`**: Users can click to select vehicles, roads, or intersections. A custom UI panel displays relevant statistics for the selected object, such as a vehicle's current and average speed, or a road's traffic count. Selected roads can be disabled and enabled, which removes them from the pathfinding network and forces vehicles to recalculate their paths.

    * **`Path Visualization`**: Selecting a vehicle renders its projected path as a ghost trail, visualizing the route it will take to its destination.

    * **`Data Logging`**: Objects now persist statistical data (thread-safe). Roads record the volume and average speed of traffic, while intersections log the number of vehicles that have passed through.

## Setup & Compilation

see [setup_and_compilation_guide.md](setup_and_compilation_guide.md) and [running_guide.md](running_guide.md).


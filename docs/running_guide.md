## Running the Simulation

After a successful compilation, the executable will be located in the build directory. The excecutable can be run using the following arguments, or the arguments can be set in a config file, which is then passed as an argument.

Examples:

`./TrafficSimulator -OSMFile ../data/maps/StNewlynEastMap.osm -NumberOfVehicles 500 -FastSimulation true -SimulationTicks 2000`

`./TrafficSimulator -ConfigFile ../data/config.txt`

Example config file:
```
# Configuration File for TrafficSimulator
# Lines starting with # are comments and will be ignored.
# Keys and values are separated by '='.

OSMFile = ../data/maps/Berkely.osm
NumberOfVehicles = 125
TicksPerSecond = 15
FastSimulation = true
SimulationTicks = 8000
```

### List of arguments
- OSMFile: Path to the OSM file to be used for the simulation. (str, eg: "../data/StNewlynEastMap.osm")
- NumberOfVehicles: Number of vehicles to be used for the simulation. (int, eg: 500)
- TicksPerSecond: Target simulation ticks per second, clamped between 5 and 120. (int, default: 60)
- FastSimulation: Whether to use fast simulation mode. (bool, eg: true)
- SimulationTicks: Number of ticks to be used for the simulation. (int, eg: 2000)
- ConfigFile: Path to the config file to be used for the simulation. (str, eg: "../data/config.txt")
- BenchmarkOutput: Path to the benchmark output file to be used for the simulation. (str, eg: "../data/profilingresults/benchmarks.csv"). If this has a value, the simulation will run using benchmarking mode.

### Running Unit Tests

To verify the integrity of the core components (Quadtree, Pathfinder, Map, Vehicle), run the integrated test suite:

1.  Navigate to the build directory:
    `cd build`
2.  Run the tests using CTest:
    `ctest --output-on-failure`

This will execute all registered tests and report any failures.

### Benchmarking
The system includes a benchmarking tool to analyze performance. To use this, you can run a simulation with `FastSimulation` enabled and specify an output file for the benchmark using the `BenchmarkOutput` argument.eg:
```bash
./TrafficSimulator -OSMFile ../data/maps/StNewlynEastMap.osm -NumberOfVehicles 500 -FastSimulation true -SimulationTicks 1000 -BenchmarkOutput ../data/profilingresults/benchmarks.csv
```

The output CSV contains timing data for each section of the simulation per tick:
```csv
Tick,TotalTick,Quadtree,Vehicles,Render
```

### Interactive GUI and Controls

The simulation features an interactive graphical user interface. You can view all available controls in-game by pressing **`H`** to open the Help Panel.

**Road Disabling Feature:**
You can dynamically modify the traffic network while the simulation is running:
1. **Select a road** by left-clicking on it (Selected roads highlight blue/red).
2. **Press `E`** to toggle its enabled/disabled state.

When a road is disabled, it is removed from the Pathfinding network. Any vehicles currently scheduled to navigate through that road will dynamically recalculate their paths in real-time to bypass the closure!
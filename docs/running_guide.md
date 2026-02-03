## Running the Simulation

After a successful compilation, the executable will be located in the build directory.

`./TrafficSimulator OSMFile(str) NumberOfVehicles(int) FastSimulation(bool, optional) SimulationTicks(int, optional, only using if FastSimulation is true)`

`./TrafficSimulator ConfigFile(str)`

Examples:

`./TrafficSimulator -OSMFile ../data/StNewlynEastMap.osm -NumberOfVehicles 500 -FastSimulation true -SimulationTicks 2000`

`./TrafficSimulator -ConfigFile ../data/config.txt`

Example config file:
```
# Configuration File for TrafficSimulator
# Lines starting with # are comments and will be ignored.
# Keys and values are separated by '='.

OSMFile = data/StNewlynEastMap.osm
NumberOfVehicles = 500
FastSimulation = true
SimulationTicks = 20000
```

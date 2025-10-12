#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Map.hpp"
#include <memory>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_osm_file> [num_vehicles]" << std::endl;
        return 1;
    }

    int numVehicles = 100;
    if (argc >= 3) {
        try {
            numVehicles = std::stoi(argv[2]);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid number of vehicles. Using default: " << numVehicles << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Number of vehicles out of range. Using default: " << numVehicles << std::endl;
        }
    }

    // Create the engine
    Engine engine(800, 600, "Traffic Simulation Prototype");

    // Load the map from the command line argument
    engine.setMap(std::make_unique<Map>(argv[1]));

    // Spawn vehicles
    engine.spawnVehicles(numVehicles);

    // Run the main loop
    engine.run();

    return 0;
}
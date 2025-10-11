#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Map.hpp"
#include <memory>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_osm_file>" << std::endl;
        return 1;
    }

    // Create the engine
    Engine engine(800, 600, "Traffic Simulation Prototype");

    // Load the map from the command line argument
    engine.setMap(std::make_unique<Map>(argv[1]));

    // Run the main loop
    engine.run();

    return 0;
}
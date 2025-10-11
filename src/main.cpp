#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Map.hpp"
#include <memory>

int main() {
    // Create the engine
    Engine engine(800, 600, "Traffic Simulation Prototype");

    // Load the map
    engine.setMap(std::make_unique<Map>("../data/CumbraeMap.osm"));

    // Run the main loop
    engine.run();

    return 0;
}
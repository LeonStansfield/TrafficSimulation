#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Map.hpp"
#include <memory>

int main() {
    // Create the engine
    Engine engine(800, 600, "Traffic Simulation Prototype");

    // Load the map
    engine.setMap(std::make_unique<Map>("../data/map.osm"));

    // Create game objects and add them to the engine
    engine.addObject(std::make_unique<Vehicle>(
        Vector2{400, 300}, Vector2{4, 2}, 2.0f, BLUE
    ));
    engine.addObject(std::make_unique<Vehicle>(
        Vector2{400, 350}, Vector2{4, 2}, 2.5f, RED
    ));
    engine.addObject(std::make_unique<Vehicle>(
        Vector2{400, 400}, Vector2{4, 2}, 1.5f, GREEN
    ));

    // Run the main loop
    engine.run();

    return 0;
}
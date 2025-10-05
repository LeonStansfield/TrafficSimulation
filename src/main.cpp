#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Map.hpp" // <-- Include the Map header
#include <memory>

int main() {
    // Create the engine
    Engine engine(800, 600, "Traffic Simulation Prototype");

    // Load the map
    engine.setMap(std::make_unique<Map>("data/map.osm"));

    // Create game objects and add them to the engine
    engine.addObject(std::make_unique<Vehicle>(
        Vector2{100, 285}, Vector2{40, 20}, 2.0f, BLUE
    ));
    engine.addObject(std::make_unique<Vehicle>(
        Vector2{200, 285}, Vector2{40, 20}, 2.5f, RED
    ));
    engine.addObject(std::make_unique<Vehicle>(
        Vector2{50, 315}, Vector2{40, 20}, 1.5f, GREEN
    ));

    // Run the main loop
    engine.run();

    return 0;
}
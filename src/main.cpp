#include "Engine.hpp"
#include "Vehicle.hpp"
#include "Map.hpp"
#include <memory>

int main() {
    Engine engine(800, 600, "Traffic Simulation Prototype");

    auto map = std::make_unique<Map>("../data/map.osm");
    engine.setMap(std::move(map));

    Map* mapPtr = engine.getMap();

    if (mapPtr) {
        for (int i = 0; i < 5000; ++i) {
            engine.addObject(std::make_unique<Vehicle>(
                mapPtr, Vector2{1, 1}, 1.5f, BLUE
            ));
        }
    }

    engine.run();

    return 0;
}
#include "Map.hpp"
#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <limits> // Required for numeric_limits

class MapHandler : public osmium::handler::Handler {
public:
    std::map<long, Vector2>& nodes;
    std::vector<Road>& roads;
    // Add references to the map boundaries
    float& minLat;
    float& maxLat;
    float& minLon;
    float& maxLon;


    MapHandler(std::map<long, Vector2>& n, std::vector<Road>& r, float& minLat, float& maxLat, float& minLon, float& maxLon)
        : nodes(n), roads(r), minLat(minLat), maxLat(maxLat), minLon(minLon), maxLon(maxLon) {}

    void node(const osmium::Node& node) {
    }

    void way(const osmium::Way& way) {
    }
};


Map::Map(const char* filename) {
    // Initialize boundaries
    minLat = std::numeric_limits<float>::max();
    maxLat = std::numeric_limits<float>::lowest();
    minLon = std::numeric_limits<float>::max();
    maxLon = std::numeric_limits<float>::lowest();

    // Read the OSM file using Osmium
}

void Map::draw() {
}
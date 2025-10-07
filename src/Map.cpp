#if defined(_WIN32)
    #define NOGDI             // All GDI defines and routines
    #define NOUSER            // All USER defines and routines
#endif

#include "Map.hpp"
#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <limits>
#include <iostream>
#include <algorithm>

class MapHandler : public osmium::handler::Handler {
public:
    std::map<long, Vector2>& nodes;
    std::vector<Road>& roads;

    MapHandler(std::map<long, Vector2>& n, std::vector<Road>& r)
        : nodes(n), roads(r) {}

    void node(const osmium::Node& node) {
        // Just store the node location, don't calculate boundaries here anymore
        nodes[static_cast<long>(node.id())] = {
            static_cast<float>(node.location().lat()),
            static_cast<float>(node.location().lon())
        };
    }

    void way(const osmium::Way& way) {
        const char* highway = way.tags().get_value_by_key("highway");
        if (highway) {
            Road road;
            for (const auto& node_ref : way.nodes()) {
                auto it = nodes.find(static_cast<long>(node_ref.ref()));
                if (it != nodes.end()) {
                    road.points.push_back(it->second);
                }
            }
            // Only add the road if it has at least two points
            if (road.points.size() >= 2) {
                roads.push_back(road);
            }
        }
    }
};

Map::Map(const char* filename) {
    minLat = std::numeric_limits<float>::max();
    maxLat = std::numeric_limits<float>::lowest();
    minLon = std::numeric_limits<float>::max();
    maxLon = std::numeric_limits<float>::lowest();

    try {
        osmium::io::Reader reader{filename, osmium::io::read_meta::no};
        // Pass only nodes and roads to the handler now
        MapHandler handler(nodes, roads);
        osmium::apply(reader, handler);
        reader.close();

        // --- NEW: Calculate boundaries AFTER loading roads ---
        // This ensures the boundary is based only on points that are actually used.
        for (const auto& road : roads) {
            for (const auto& point : road.points) {
                float lat = point.x;
                float lon = point.y;
                if (lat < minLat) minLat = lat;
                if (lat > maxLat) maxLat = lat;
                if (lon < minLon) minLon = lon;
                if (lon > maxLon) maxLon = lon;
            }
        }

        std::cout << "Map data loaded successfully." << std::endl;
        std::cout << "Number of nodes processed: " << nodes.size() << std::endl;
        std::cout << "Number of roads created: " << roads.size() << std::endl;
        std::cout << "Map boundaries (from used nodes):" << std::endl;
        std::cout << "  Min Latitude: " << minLat << std::endl;
        std::cout << "  Max Latitude: " << maxLat << std::endl;
        std::cout << "  Min Longitude: " << minLon << std::endl;
        std::cout << "  Max Longitude: " << maxLon << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error loading map data: " << e.what() << std::endl;
    }

    // Correctly calculate world dimensions while preserving aspect ratio
    float lon_range = maxLon - minLon;
    float lat_range = maxLat - minLat;

    if (lon_range == 0 || lat_range == 0) {
        worldWidth = 1000;
        worldHeight = 1000;
        return;
    }

    const float base_size = 1000.0f; // Define a base size for the longer dimension

    if (lon_range > lat_range) {
        worldWidth = base_size;
        worldHeight = base_size * (lat_range / lon_range);
    } else {
        worldHeight = base_size;
        worldWidth = base_size * (lon_range / lat_range);
    }
}

Vector2 Map::convertLatLonToWorld(Vector2 latLon) {
    // Handle potential division by zero if map is a single point
    float lon_range = (maxLon - minLon);
    float lat_range = (maxLat - minLat);
    if (lon_range == 0 || lat_range == 0) {
        return { worldWidth / 2.0f, worldHeight / 2.0f };
    }

    return {
        (latLon.y - minLon) * (worldWidth / lon_range),
        (maxLat - latLon.x) * (worldHeight / lat_range)
    };
}


void Map::draw() {
    // Draw the world boundary
    DrawRectangleLines(0, 0, static_cast<int>(worldWidth), static_cast<int>(worldHeight), RED);

    // Draw a marker at the center
    float centerX = worldWidth / 2.0f;
    float centerY = worldHeight / 2.0f;
    DrawLine(centerX - 15, centerY, centerX + 15, centerY, YELLOW);
    DrawLine(centerX, centerY - 15, centerX, centerY + 15, YELLOW);

    // Draw the roads
    for (const auto& road : roads) {
        for (size_t i = 1; i < road.points.size(); ++i) {
            Vector2 start_world = convertLatLonToWorld(road.points[i-1]);
            Vector2 end_world = convertLatLonToWorld(road.points[i]);
            DrawLineV(start_world, end_world, GRAY);
        }
    }
}
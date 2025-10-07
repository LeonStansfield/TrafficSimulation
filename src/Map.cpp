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
#include <set>

class MapHandler : public osmium::handler::Handler {
public:
    std::map<long, Vector2>& nodes;
    std::vector<Road>& roads;

    // A set of highway tags that we consider drivable roads
    const std::set<std::string> drivable_tags = {
        "motorway", "trunk", "primary", "secondary", "tertiary", "unclassified",
        "residential", "motorway_link", "trunk_link", "primary_link",
        "secondary_link", "tertiary_link", "living_street", "service"
    };

    MapHandler(std::map<long, Vector2>& n, std::vector<Road>& r)
        : nodes(n), roads(r) {}

    void node(const osmium::Node& node) {
        nodes[static_cast<long>(node.id())] = {
            static_cast<float>(node.location().lat()),
            static_cast<float>(node.location().lon())
        };
    }

    void way(const osmium::Way& way) {
        const char* highway_tag = way.tags().get_value_by_key("highway");

        // Only proceed if the highway tag exists and is in our set of drivable tags
        if (highway_tag && drivable_tags.count(highway_tag)) {
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
        MapHandler handler(nodes, roads);
        osmium::apply(reader, handler);
        reader.close();

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

    float lon_range = maxLon - minLon;
    float lat_range = maxLat - minLat;

    if (lon_range == 0 || lat_range == 0) {
        worldWidth = 1000;
        worldHeight = 1000;
        return;
    }

    const float base_size = 1000.0f;

    if (lon_range > lat_range) {
        worldWidth = base_size;
        worldHeight = base_size * (lat_range / lon_range);
    } else {
        worldHeight = base_size;
        worldWidth = base_size * (lon_range / lat_range);
    }
}

Vector2 Map::convertLatLonToWorld(Vector2 latLon) {
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
    DrawRectangleLines(0, 0, static_cast<int>(worldWidth), static_cast<int>(worldHeight), RED);

    float centerX = worldWidth / 2.0f;
    float centerY = worldHeight / 2.0f;
    DrawLine(centerX - 15, centerY, centerX + 15, centerY, YELLOW);
    DrawLine(centerX, centerY - 15, centerX, centerY + 15, YELLOW);

    for (const auto& road : roads) {
        for (size_t i = 1; i < road.points.size(); ++i) {
            Vector2 start_world = convertLatLonToWorld(road.points[i-1]);
            Vector2 end_world = convertLatLonToWorld(road.points[i]);
            DrawLineV(start_world, end_world, GRAY);
        }
    }
}
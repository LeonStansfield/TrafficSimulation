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
    float& minLat;
    float& maxLat;
    float& minLon;
    float& maxLon;

    MapHandler(std::map<long, Vector2>& n, std::vector<Road>& r, float& minLat, float& maxLat, float& minLon, float& maxLon)
        : nodes(n), roads(r), minLat(minLat), maxLat(maxLat), minLon(minLon), maxLon(maxLon) {}

    void node(const osmium::Node& node) {
        nodes[static_cast<long>(node.id())] = {
            static_cast<float>(node.location().lat()),
            static_cast<float>(node.location().lon())
        };

        float lat = static_cast<float>(node.location().lat());
        float lon = static_cast<float>(node.location().lon());

        if (lat < minLat) minLat = lat;
        if (lat > maxLat) maxLat = lat;
        if (lon < minLon) minLon = lon;
        if (lon > maxLon) maxLon = lon;
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
            roads.push_back(road);
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
        MapHandler handler(nodes, roads, minLat, maxLat, minLon, maxLon);
        osmium::apply(reader, handler);
        reader.close();

        std::cout << "Map data loaded successfully." << std::endl;
        std::cout << "Number of nodes read: " << nodes.size() << std::endl;
        std::cout << "Number of roads read: " << roads.size() << std::endl;
        std::cout << "Map boundaries:" << std::endl;
        std::cout << "  Min Latitude: " << minLat << std::endl;
        std::cout << "  Max Latitude: " << maxLat << std::endl;
        std::cout << "  Min Longitude: " << minLon << std::endl;
        std::cout << "  Max Longitude: " << maxLon << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error loading map data: " << e.what() << std::endl;
    }
}

void Map::draw() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Avoid division by zero if the map has no size
    if (maxLon - minLon == 0 || maxLat - minLat == 0) {
        return;
    }

    // Calculate the scale factors to fit the map to the screen
    float scaleX = static_cast<float>(screenWidth) / (maxLon - minLon);
    float scaleY = static_cast<float>(screenHeight) / (maxLat - minLat);

    // Use the smaller scale factor to ensure the entire map is visible
    float scale = std::min(scaleX, scaleY);

    // Calculate the dimensions of the scaled map
    float scaledMapWidth = (maxLon - minLon) * scale;
    float scaledMapHeight = (maxLat - minLat) * scale;

    // Calculate the offsets to center the map on the screen
    float offsetX = (static_cast<float>(screenWidth) - scaledMapWidth);
    float offsetY = (static_cast<float>(screenHeight) - scaledMapHeight);

    for (const auto& road : roads) {
        for (size_t i = 1; i < road.points.size(); ++i) {
            Vector2 start_latlon = road.points[i - 1];
            Vector2 end_latlon = road.points[i];

            // Apply the scaling and offset
            Vector2 start_screen = {
                (start_latlon.y - minLon) * scale + offsetX,
                (maxLat - start_latlon.x) * scale + offsetY
            };

            Vector2 end_screen = {
                (end_latlon.y - minLon) * scale + offsetX,
                (maxLat - end_latlon.x) * scale + offsetY
            };

            DrawLineV(start_screen, end_screen, GRAY);
        }
    }
}
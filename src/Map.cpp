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
#include <cmath>
#include <random>

class MapHandler : public osmium::handler::Handler {
public:
    std::map<long, Vector2>& nodes;
    std::vector<std::vector<long>>& ways;

    const std::set<std::string> drivable_tags = {
        "motorway", "trunk", "primary", "secondary", "tertiary", "unclassified",
        "residential", "motorway_link", "trunk_link", "primary_link",
        "secondary_link", "tertiary_link", "living_street", "service"
    };

    MapHandler(std::map<long, Vector2>& n, std::vector<std::vector<long>>& w)
        : nodes(n), ways(w) {}

    void node(const osmium::Node& node) {
        nodes[static_cast<long>(node.id())] = {
            static_cast<float>(node.location().lat()),
            static_cast<float>(node.location().lon())
        };
    }

    void way(const osmium::Way& way) {
        const char* highway_tag = way.tags().get_value_by_key("highway");

        if (highway_tag && drivable_tags.count(highway_tag)) {
            // If it's a service road, check if it's a driveway. If so, skip it.
            if (strcmp(highway_tag, "service") == 0) {
                const char* service_tag = way.tags().get_value_by_key("service");
                if (service_tag && strcmp(service_tag, "driveway") == 0) {
                    return; // Skip this way
                }
            }

            std::vector<long> way_nodes;
            for (const auto& node_ref : way.nodes()) {
                way_nodes.push_back(static_cast<long>(node_ref.ref()));
            }
            if (way_nodes.size() >= 2) {
                ways.push_back(way_nodes);
            }
        }
    }
};

Map::Map(const char* filename) {
    std::vector<std::vector<long>> ways_temp;

    try {
        osmium::io::Reader reader{filename, osmium::io::read_meta::no};
        MapHandler handler(nodes, ways_temp);
        osmium::apply(reader, handler);
        reader.close();

    } catch (const std::exception& e) {
        std::cerr << "Error loading map data: " << e.what() << std::endl;
        return;
    }

    // Determine bounds from road nodes only
    std::set<long> road_node_ids;
    for (const auto& way : ways_temp) {
        for (long node_id : way) {
            road_node_ids.insert(node_id);
        }
    }

    minLat = std::numeric_limits<float>::max();
    maxLat = std::numeric_limits<float>::lowest();
    minLon = std::numeric_limits<float>::max();
    maxLon = std::numeric_limits<float>::lowest();

    for (long node_id : road_node_ids) {
        const auto& node = nodes[node_id];
        float lat = node.x;
        float lon = node.y;
        if (lat < minLat) minLat = lat;
        if (lat > maxLat) maxLat = lat;
        if (lon < minLon) minLon = lon;
        if (lon > maxLon) maxLon = lon;
    }

    // Realistic scaling
    float centerLat = minLat + (maxLat - minLat) / 2.0f;
    worldWidth = std::abs(maxLon - minLon) * 111320.0f * std::cos(centerLat * M_PI / 180.0f);
    worldHeight = std::abs(maxLat - minLat) * 110574.0f;


    // Find intersections
    std::map<long, int> node_counts;
    for (const auto& way : ways_temp) {
        for (size_t i = 0; i < way.size(); ++i) {
            node_counts[way[i]]++;
        }
    }

    for (const auto& pair : node_counts) {
        if (pair.second > 1) {
            intersections[pair.first] = {pair.first, convertLatLonToWorld(nodes[pair.first])};
        }
    }

    for (const auto& way : ways_temp) {
        if(intersections.find(way.front()) == intersections.end()){
             intersections[way.front()] = {way.front(), convertLatLonToWorld(nodes[way.front()])};
        }
        if(intersections.find(way.back()) == intersections.end()){
             intersections[way.back()] = {way.back(), convertLatLonToWorld(nodes[way.back()])};
        }
    }

    // Build roads
    for (const auto& way : ways_temp) {
        Road current_road;
        size_t last_intersection_index = 0;
        for (size_t i = 0; i < way.size(); ++i) {
            if (intersections.count(way[i])) {
                if (i > last_intersection_index) {
                    current_road.points.push_back(intersections[way[last_intersection_index]].position);
                    for (size_t j = last_intersection_index + 1; j < i; ++j) {
                        current_road.points.push_back(convertLatLonToWorld(nodes[way[j]]));
                    }
                    current_road.points.push_back(intersections[way[i]].position);
                    roads.push_back(current_road);
                    current_road.points.clear();
                }
                last_intersection_index = i;
            }
        }
    }

    std::cout << "Map data loaded successfully." << std::endl;
    std::cout << "Number of nodes processed: " << nodes.size() << std::endl;
    std::cout << "Number of intersections created: " << intersections.size() << std::endl;
    std::cout << "Number of roads created: " << roads.size() << std::endl;
    std::cout << "World dimensions (meters): " << worldWidth << " x " << worldHeight << std::endl;
}

Vector2 Map::convertLatLonToWorld(Vector2 latLon) {
    float x = (latLon.y - minLon) * 111320.0f * std::cos( (minLat + (maxLat - minLat) / 2.0f) * M_PI / 180.0f);
    float y = (maxLat - latLon.x) * 110574.0f;
    return {x, y};
}

void Map::draw() {
    DrawRectangleLines(0, 0, static_cast<int>(worldWidth), static_cast<int>(worldHeight), RED);

    for (const auto& road : roads) {
        for (size_t i = 1; i < road.points.size(); ++i) {
            DrawLineV(road.points[i - 1], road.points[i], GRAY);
        }
    }

    for (const auto& pair : intersections) {
        DrawCircleV(pair.second.position, 5.0f, BLUE);
    }
}

const std::vector<Road>& Map::getRoads() const {
    return roads;
}


const Road* Map::getClosestRoad(Vector2 position) const {
    const Road* closestRoad = nullptr;
    float minDistanceSq = std::numeric_limits<float>::max();

    for (const auto& road : roads) {
        for (const auto& point : road.points) {
            float distSq = (point.x - position.x) * (point.x - position.x) + (point.y - position.y) * (point.y - position.y);
            if (distSq < minDistanceSq) {
                minDistanceSq = distSq;
                closestRoad = &road;
            }
        }
    }
    return closestRoad;
}

const Road* Map::getRandomConnectedRoad(const Road& currentRoad, std::mt19937& rng) const {
    if (roads.empty() || currentRoad.points.empty()) return nullptr;

    Vector2 endPoint = currentRoad.points.back();
    std::vector<const Road*> connectedRoads;

    for (const auto& road : roads) {
        if (&road == &currentRoad || road.points.empty()) continue;
        // check if road starts where current ends
        if (road.points.front().x == endPoint.x && road.points.front().y == endPoint.y) {
            connectedRoads.push_back(&road);
        }
    }

    if (connectedRoads.empty()) return nullptr;
    if (connectedRoads.size() == 1) return connectedRoads.front();

    // random selection using the passed-in generator
    std::uniform_int_distribution<> dist(0, static_cast<int>(connectedRoads.size()) - 1);
    return connectedRoads[dist(rng)];
}
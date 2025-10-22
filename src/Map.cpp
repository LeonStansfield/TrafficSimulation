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
#include "raymath.h"
#include <algorithm> // for std::min/max

// Struct to hold temporary way data during parsing
struct WayData {
    std::vector<long> node_ids;
    bool isOneWay;
};

class MapHandler : public osmium::handler::Handler {
public:
    std::map<long, Vector2>& nodes;
    std::vector<WayData>& ways; // Updated to use WayData

    const std::set<std::string> drivable_tags = {
        "motorway", "trunk", "primary", "secondary", "tertiary", "unclassified",
        "residential", "motorway_link", "trunk_link", "primary_link",
        "secondary_link", "tertiary_link", "living_street", "service"
    };

    MapHandler(std::map<long, Vector2>& n, std::vector<WayData>& w)
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

            // Check for oneway tag
            const char* oneway_tag = way.tags().get_value_by_key("oneway");
            bool oneWay = (oneway_tag && (strcmp(oneway_tag, "yes") == 0 || strcmp(oneway_tag, "1") == 0));

            std::vector<long> way_nodes;
            for (const auto& node_ref : way.nodes()) {
                way_nodes.push_back(static_cast<long>(node_ref.ref()));
            }
            if (way_nodes.size() >= 2) {
                ways.push_back({way_nodes, oneWay}); // Store WayData
            }
        }
    }
};

// Helper function to create an offset polyline for two-way roads
static std::vector<Vector2> offsetPolyline(const std::vector<Vector2>& polyline, float offset) {
    if (polyline.size() < 2) return polyline;

    std::vector<Vector2> offsetPoints;
    offsetPoints.reserve(polyline.size()); 

    for (size_t i = 0; i < polyline.size() - 1; ++i) {
        Vector2 p1 = polyline[i];
        Vector2 p2 = polyline[i + 1];

        Vector2 dir = Vector2Normalize(Vector2Subtract(p2, p1));
        Vector2 normal = { -dir.y, dir.x }; // Perpendicular
        Vector2 offsetVec = Vector2Scale(normal, offset);

        if (i == 0) {
            offsetPoints.push_back(Vector2Add(p1, offsetVec));
        }
        offsetPoints.push_back(Vector2Add(p2, offsetVec));
    }
    return offsetPoints;
}

static Vector2 GetClosestPointOnLineSegment(Vector2 point, Vector2 p1, Vector2 p2) {
    Vector2 segmentVec = Vector2Subtract(p2, p1);
    Vector2 toPointVec = Vector2Subtract(point, p1);

    float segmentLenSqr = Vector2LengthSqr(segmentVec);
    if (segmentLenSqr == 0.0f) return p1; // Segment is just a point

    // Project toPointVec onto segmentVec
    float t = Vector2DotProduct(toPointVec, segmentVec) / segmentLenSqr;

    // Clamp t to [0, 1] to stay on the segment
    t = std::clamp(t, 0.0f, 1.0f);

    // Calculate the closest point on the segment
    return Vector2Add(p1, Vector2Scale(segmentVec, t));
}

// Draws a small triangle arrow to indicate direction
static void drawArrow(Vector2 midPoint, Vector2 direction, float size, Color color) {
    Vector2 p1 = Vector2Add(midPoint, Vector2Scale(direction, size / 2.0f)); // Arrow tip
    Vector2 normal = { -direction.y, direction.x };
    
    Vector2 p2 = Vector2Subtract(midPoint, Vector2Scale(direction, size / 2.0f));
    p2 = Vector2Add(p2, Vector2Scale(normal, size / 3.0f)); // Left base
    
    Vector2 p3 = Vector2Subtract(midPoint, Vector2Scale(direction, size / 2.0f));
    p3 = Vector2Add(p3, Vector2Scale(normal, -size / 3.0f)); // Right base
    
    DrawTriangle(p1, p2, p3, color);
}

Map::Map(const char* filename) {
    std::vector<WayData> ways_temp; // Use WayData

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
        for (long node_id : way.node_ids) { // Updated
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
        for (size_t i = 0; i < way.node_ids.size(); ++i) { // Updated
            node_counts[way.node_ids[i]]++; // Updated
        }
    }

    for (const auto& pair : node_counts) {
        if (pair.second > 1) {
            intersections[pair.first] = {pair.first, convertLatLonToWorld(nodes[pair.first])};
        }
    }

    // Also add all way endpoints as intersections
    for (const auto& way : ways_temp) {
        if(intersections.find(way.node_ids.front()) == intersections.end()){ // Updated
             intersections[way.node_ids.front()] = {way.node_ids.front(), convertLatLonToWorld(nodes[way.node_ids.front()])}; // Updated
        }
        if(intersections.find(way.node_ids.back()) == intersections.end()){ // Updated
             intersections[way.node_ids.back()] = {way.node_ids.back(), convertLatLonToWorld(nodes[way.node_ids.back()])}; // Updated
        }
    }

    // Build roads
    float roadOffset = 3.5f; // Approx. lane width in meters
    roads.clear();
    baseRoadSegments.clear();
    baseSegmentOneWay.clear();

    for (const auto& way : ways_temp) {
        size_t last_intersection_index = 0;
        for (size_t i = 1; i < way.node_ids.size(); ++i) { // Start from 1
            
            // Split segment if we hit an intersection or the end of the way
            if (intersections.count(way.node_ids[i]) || i == way.node_ids.size() - 1) {
                
                long startNodeId = way.node_ids[last_intersection_index];
                long endNodeId = way.node_ids[i];

                if (intersections.find(startNodeId) == intersections.end() || intersections.find(endNodeId) == intersections.end()) {
                    last_intersection_index = i;
                    continue; 
                }
                
                std::vector<Vector2> base_points;
                for (size_t j = last_intersection_index; j <= i; ++j) {
                    base_points.push_back(convertLatLonToWorld(nodes[way.node_ids[j]]));
                }
                
                base_points.front() = intersections[startNodeId].position;
                base_points.back() = intersections[endNodeId].position;

                // Store this base polyline for clean rendering
                std::pair<long, long> segmentId = {
                    std::min(startNodeId, endNodeId),
                    std::max(startNodeId, endNodeId)
                };

                if (baseRoadSegments.find(segmentId) == baseRoadSegments.end()) {
                    baseRoadSegments[segmentId] = base_points;
                    baseSegmentOneWay[segmentId] = way.isOneWay;
                }
                
                // Create the forward road
                Road forward_road;
                forward_road.fromIntersectionId = startNodeId;
                forward_road.toIntersectionId = endNodeId;
                forward_road.isOneWay = way.isOneWay;
                
                if (!way.isOneWay) {
                    forward_road.points = offsetPolyline(base_points, roadOffset);
                } else {
                    forward_road.points = base_points;
                }
                roads.push_back(forward_road);
                
                // If it's not one-way, create the backward road
                if (!way.isOneWay) {
                    Road backward_road;
                    backward_road.fromIntersectionId = endNodeId;
                    backward_road.toIntersectionId = startNodeId;
                    backward_road.isOneWay = false; // Part of two-way

                    std::vector<Vector2> reversed_base_points = base_points;
                    std::reverse(reversed_base_points.begin(), reversed_base_points.end());
                    backward_road.points = offsetPolyline(reversed_base_points, roadOffset);
                    roads.push_back(backward_road);
                }
                
                last_intersection_index = i;
            }
        }
    }

    // Build the adjacency list
    outgoingRoads.clear();
    for (const auto& road : roads) {
        outgoingRoads[road.fromIntersectionId].push_back(&road);
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

void Map::draw(bool debug) {
    DrawRectangleLines(0, 0, static_cast<int>(worldWidth), static_cast<int>(worldHeight), RED);

    if (debug) {
        // DEBUG VIEW: Draw the *actual* vehicle paths (offset lines)
        for (const auto& road : roads) {
            Color roadColor = road.isOneWay ? ColorAlpha(SKYBLUE, 0.7f) : ColorAlpha(WHITE, 0.7f);
            
            for (size_t i = 0; i < road.points.size(); ++i) {
                // Draw dots for every point
                DrawCircleV(road.points[i], 1.5f, roadColor);
                
                // Draw faint lines connecting them
                if (i > 0) {
                    DrawLineV(road.points[i - 1], road.points[i], ColorAlpha(roadColor, 0.3f));
                }
            }
        }
    } else {
        // NORMAL VIEW: Draw individual road segments
        for (const auto& pair : baseRoadSegments) {
            const std::vector<Vector2>& points = pair.second;
            bool isOneWay = baseSegmentOneWay.at(pair.first);
            
            Color roadColor = GRAY;
            for (size_t i = 1; i < points.size(); ++i) {
                DrawLineV(points[i - 1], points[i], roadColor);
            }

            // Draw direction arrows on top of the roads
            if (points.size() >= 2) {
                // Find the middle segment of the polyline
                size_t midIndex = (points.size() - 1) / 2;
                
                Vector2 start = points[midIndex];
                Vector2 end = points[midIndex + 1]; // Safe because points.size() >= 2
                
                Vector2 midPoint = Vector2Scale(Vector2Add(start, end), 0.5f);
                Vector2 dir = Vector2Subtract(end, start);

                // Check for zero-length segment
                if (Vector2LengthSqr(dir) < 0.001f) {
                    continue; // Skip arrow for tiny segments
                }
                dir = Vector2Normalize(dir);

                if (isOneWay) {
                    // One-way: Draw one arrow in the center
                    drawArrow(midPoint, dir, 8.0f, BLUE);
                } else {
                    // Two-way: Draw two arrows, offset from the center
                    Vector2 normal = { -dir.y, dir.x };
                    Vector2 leftArrowPos = Vector2Add(midPoint, Vector2Scale(normal, 2.5f));
                    Vector2 rightArrowPos = Vector2Subtract(midPoint, Vector2Scale(normal, 2.5f));
                    
                    drawArrow(leftArrowPos, dir, 7.0f, BLUE);
                    drawArrow(rightArrowPos, Vector2Scale(dir, -1.0f), 7.0f, BLUE);
                }
            }
        }
    }

    // Draw intersections on top of everything
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
        for (size_t i = 0; i < road.points.size() - 1; ++i) {
            Vector2 p1 = road.points[i];
            Vector2 p2 = road.points[i+1];
            Vector2 closestPoint = GetClosestPointOnLineSegment(position, p1, p2);
            float distSq = Vector2DistanceSqr(position, closestPoint);
            
            if (distSq < minDistanceSq) {
                minDistanceSq = distSq;
                closestRoad = &road;
            }
        }
    }
    return closestRoad;
}

const Road* Map::getRandomOutgoingRoad(long fromIntersectionId, std::mt19937& rng) const {
    auto it = outgoingRoads.find(fromIntersectionId);
    if (it == outgoingRoads.end() || it->second.empty()) {
        return nullptr; // No outgoing roads (dead end)
    }

    std::uniform_int_distribution<> dist(0, static_cast<int>(it->second.size()) - 1);
    return it->second[dist(rng)];
}
#pragma once

#include <vector>
#include <memory>
#include "Object.hpp"
#include "Map.hpp"
#include "Quadtree.hpp"
#include "Pathfinder.hpp"

class Engine {
private:
    int screenWidth;
    int screenHeight;
    Camera2D camera;
    
    std::vector<std::unique_ptr<Object>> objects;
    std::unique_ptr<Map> map;
    std::unique_ptr<Quadtree> quadtree;
    std::unique_ptr<Pathfinder> pathfinder;

    bool debug;

public:
    Engine(int width, int height, const char* title);
    ~Engine();

    void setMap(std::unique_ptr<Map> newMap);
    void addObject(std::unique_ptr<Object> object);
    void run();
    void spawnVehicles(int count);
};
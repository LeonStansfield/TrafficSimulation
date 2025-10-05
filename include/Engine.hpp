#pragma once

#include "raylib.h"
#include "Object.hpp"
#include "Map.hpp"
#include <vector>
#include <memory>

class Engine {
private:
    std::vector<std::unique_ptr<Object>> objects;
    std::unique_ptr<Map> map;
    int screenWidth;
    int screenHeight;

public:
    // Constructor and Destructor
    Engine(int width, int height, const char* title);
    ~Engine();

    // Public methods
    void setMap(std::unique_ptr<Map> newMap);
    void addObject(std::unique_ptr<Object> object);
    void run();
};
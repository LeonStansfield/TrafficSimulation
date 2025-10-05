#pragma once

#include "raylib.h"
#include "Object.hpp"
#include <vector>
#include <memory>

class Engine {
private:
    std::vector<std::unique_ptr<Object>> objects;
    int screenWidth;
    int screenHeight;

public:
    // Constructor and Destructor
    Engine(int width, int height, const char* title);
    ~Engine();

    // Public methods
    void addObject(std::unique_ptr<Object> object);
    void run();
};
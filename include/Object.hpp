#pragma once

class Object {
public:
    // Virtual destructor is crucial for base classes.
    virtual ~Object() = default;

    // Pure virtual functions that all derived classes must implement.
    virtual void update() = 0;
    virtual void draw(bool debug) = 0;
};
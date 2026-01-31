#pragma once

#include "InputController.hpp"
#include "Renderer.hpp"
#include "Simulation.hpp"
#include "raylib.h"

class Gui {
public:
  Gui();

  void draw(const Simulation &simulation, const InputController &input,
            const Renderer &renderer);
};

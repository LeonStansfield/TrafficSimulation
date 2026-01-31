#include "Engine.hpp"
#include <iostream>

Engine::Engine(int width, int height, const char *title)
    : renderer(width, height, title) {}

Engine::~Engine() {}

void Engine::setMap(std::unique_ptr<Map> newMap) {

  simulation.setMap(std::move(newMap));
  renderer.resetCamera(simulation.getMap());
}

void Engine::addObject(std::unique_ptr<Object> object) {
  simulation.addObject(std::move(object));
}

void Engine::run() {
  while (!WindowShouldClose()) {

    // Input Handling
    input.handleInput(renderer.getCamera(), simulation);

    if (IsKeyPressed(KEY_R)) {
      renderer.resetCamera(simulation.getMap());
    }

    if (IsKeyPressed(KEY_Q)) {
      renderer.cycleDrawMode();
    }

    // Update
    simulation.update(GetFrameTime());

    // Draw
    renderer.beginDrawing();

    renderer.drawSimulation(simulation);
    gui.draw(simulation, input, renderer);

    renderer.endDrawing();
  }
}

void Engine::runFast(int ticks) {
  if (!simulation.getMap()) {
    std::cerr << "Map is not set. Aborting fast run." << std::endl;
    return;
  }

  const float FIXED_DELTA_TIME = 1.0f / 60.0f;

  for (int i = 0; i < ticks; ++i) {
    if (i % (ticks / 100) == 0) {
      std::cout << "\rSimulation progress: " << (i * 100 / ticks) << "%"
                << std::flush;
    }
    simulation.update(FIXED_DELTA_TIME);
  }
  std::cout << "\rSimulation progress: 100%." << std::endl;
}

void Engine::spawnVehicles(int count) { simulation.spawnVehicles(count); }
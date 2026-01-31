#include "Quadtree.hpp"

Quadtree::Quadtree(Rectangle boundary, int capacity)
    : boundary(boundary), capacity(capacity), divided(false) {}

Quadtree::~Quadtree() { clear(); }

void Quadtree::subdivide() {
  float x = boundary.x;
  float y = boundary.y;
  float w = boundary.width / 2.0f;
  float h = boundary.height / 2.0f;

  northeast = std::make_unique<Quadtree>(Rectangle{x + w, y, w, h}, capacity);
  northwest = std::make_unique<Quadtree>(Rectangle{x, y, w, h}, capacity);
  southeast =
      std::make_unique<Quadtree>(Rectangle{x + w, y + h, w, h}, capacity);
  southwest = std::make_unique<Quadtree>(Rectangle{x, y + h, w, h}, capacity);

  divided = true;
}

bool Quadtree::insert(Vehicle *vehicle) {
  if (!CheckCollisionPointRec(vehicle->getPosition(), boundary)) {
    return false;
  }

  if (vehicles.size() < capacity) {
    vehicles.push_back(vehicle);
    return true;
  }

  if (!divided) {
    subdivide();
  }

  if (northeast->insert(vehicle))
    return true;
  if (northwest->insert(vehicle))
    return true;
  if (southeast->insert(vehicle))
    return true;
  if (southwest->insert(vehicle))
    return true;

  return false;
}

std::vector<Vehicle *> Quadtree::query(Rectangle range) const {
  std::vector<Vehicle *> found;

  if (!CheckCollisionRecs(boundary, range)) {
    return found;
  }

  for (Vehicle *v : vehicles) {
    if (CheckCollisionPointRec(v->getPosition(), range)) {
      found.push_back(v);
    }
  }

  if (divided) {
    std::vector<Vehicle *> nw = northwest->query(range);
    found.insert(found.end(), nw.begin(), nw.end());
    std::vector<Vehicle *> ne = northeast->query(range);
    found.insert(found.end(), ne.begin(), ne.end());
    std::vector<Vehicle *> sw = southwest->query(range);
    found.insert(found.end(), sw.begin(), sw.end());
    std::vector<Vehicle *> se = southeast->query(range);
    found.insert(found.end(), se.begin(), se.end());
  }

  return found;
}

void Quadtree::clear() {
  vehicles.clear();
  northwest.reset();
  northeast.reset();
  southwest.reset();
  southeast.reset();
  divided = false;
}

void Quadtree::draw(const Camera2D &camera) const {
  DrawRectangleLinesEx(boundary, 0.5f / camera.zoom, GRAY);

  if (divided) {
    northwest->draw(camera);
    northeast->draw(camera);
    southwest->draw(camera);
    southeast->draw(camera);
  }
}
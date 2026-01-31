#include "Gui.hpp"

Gui::Gui() {}

void Gui::draw(const Simulation &simulation, const InputController &input,
               const Renderer &renderer) {
  const Font &font = renderer.getFont();
  DrawMode currentMode = renderer.getDrawMode();
  SelectionType selectionType = input.getSelectionType();

  // Define style constants
  const int boxX = 20;
  const int boxY = 20;
  const int width = 260;
  const int padding = 15;
  const int lineSpacing = 24;
  const float fontSize = 16.0f;

  Color panelColor = {20, 25, 30, 230};
  Color borderColor = {60, 70, 80, 255};
  Color labelColor = GRAY;
  Color valueColor = RAYWHITE;
  Color accentColor = SKYBLUE;

  // Calculate dynamic height
  int contentHeight = 0;

  // System Status section height
  contentHeight += 10 + 20;

  // Selection section height
  if (selectionType != SelectionType::NONE) {
    contentHeight += 15;
    contentHeight += 10 + 20;
    contentHeight += lineSpacing * 3;
  }

  int totalHeight = padding * 2 + contentHeight;

  // Draw Panel
  DrawRectangle(boxX, boxY, width, totalHeight, panelColor);
  DrawRectangleLines(boxX, boxY, width, totalHeight, borderColor);

  float cursorX = (float)boxX + padding;
  float cursorY = (float)boxY + padding;

  // Helper lambda for using custom font
  auto drawText = [&](const char *text, float x, float y, float size,
                      Color col) {
    DrawTextEx(font, text, {x, y}, size, 1.0f, col);
  };

  // --- System Status ---
  drawText("SIMULATION MODE", cursorX, cursorY, fontSize, labelColor);
  cursorY += 14;

  const char *modeText = "NORMAL";
  Color modeColor = accentColor;

  switch (currentMode) {
  case DrawMode::NORMAL:
    modeText = "NORMAL";
    modeColor = accentColor;
    break;
  case DrawMode::DEBUG:
    modeText = "DEBUG";
    modeColor = ORANGE;
    break;
  case DrawMode::HEATMAP:
    modeText = "HEATMAP";
    modeColor = RED;
    break;
  }

  drawText(modeText, cursorX, cursorY, fontSize, modeColor);
  cursorY += 28;

  // --- Selection Info ---
  if (selectionType != SelectionType::NONE) {
    DrawLine((int)cursorX, (int)cursorY, boxX + width - padding, (int)cursorY,
             borderColor);
    cursorY += 12;

    if (selectionType == SelectionType::VEHICLE) {
      const Vehicle *v =
          dynamic_cast<const Vehicle *>(input.getSelectedVehicle());
      if (v) {
        drawText("SELECTED: VEHICLE", cursorX, cursorY, fontSize, labelColor);
        cursorY += 18;

        drawText("Current Speed:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%.1f m/s", v->getSpeed()), cursorX + 100,
                 cursorY - 2, fontSize, valueColor);
        cursorY += lineSpacing;

        drawText("Avg Speed:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%.1f m/s", v->getAverageSpeed()), cursorX + 100,
                 cursorY - 2, fontSize, valueColor);
      }
    } else if (selectionType == SelectionType::INTERSECTION) {
      const Intersection *i = input.getSelectedIntersection();
      if (i) {
        drawText("SELECTED: INTERSECTION", cursorX, cursorY, fontSize,
                 labelColor);
        cursorY += 18;

        drawText("ID:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("#%ld", i->id), cursorX + 100, cursorY - 2,
                 fontSize, valueColor);
        cursorY += lineSpacing;

        drawText("Visits:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%ld", i->stats.vehiclesVisited), cursorX + 100,
                 cursorY - 2, fontSize, valueColor);
      }
    } else if (selectionType == SelectionType::ROAD) {
      const Road *r = input.getSelectedRoad();
      if (r) {
        drawText("SELECTED: ROAD", cursorX, cursorY, fontSize, labelColor);
        cursorY += 18;

        drawText("Traffic:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%ld", r->stats.vehiclesPassed), cursorX + 100,
                 cursorY - 2, fontSize, valueColor);
        cursorY += lineSpacing;

        drawText("Avg Speed:", cursorX, cursorY, fontSize, labelColor);
        drawText(TextFormat("%.1f m/s", r->stats.getAverageSpeed()),
                 cursorX + 100, cursorY - 2, fontSize, valueColor);
        cursorY += lineSpacing;

        drawText("Type:", cursorX, cursorY, fontSize, labelColor);
        drawText(r->isOneWay ? "One Way" : "Two Way", cursorX + 100,
                 cursorY - 2, fontSize, valueColor);
      }
    }
  }
}

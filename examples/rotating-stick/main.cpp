#include "raylib.h"
#include "raymath.h"

int main() {
  constexpr int screenWidth = 1200;
  constexpr int screenHeight = 800;
  constexpr Vector2 center =
      Vector2{.x = screenWidth / 2, .y = screenHeight / 2};
  constexpr int ballRadius = 20;
  constexpr int stickLength = 200;
  constexpr int stickThickness = 3;
  constexpr float angularAcceleration = 0.01 * DEG2RAD;
  constexpr float maxAngularVelocity = 10 * DEG2RAD;
  float angularVelocity = 0;
  float angle = 0;

  InitWindow(screenWidth, screenHeight, "Rotating Stick");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    angularVelocity += angularAcceleration;
    if (angularVelocity > maxAngularVelocity) {
      angularVelocity = maxAngularVelocity;
    }
    angle += angularVelocity;

    Vector2 offset = Vector2Rotate(Vector2{stickLength / 2, 0}, angle);

    Vector2 start = Vector2Subtract(center, offset);
    Vector2 end = Vector2Add(center, offset);

    DrawLineEx(start, end, stickThickness, WHITE);
    DrawCircleV(start, ballRadius, WHITE);
    DrawCircleV(end, ballRadius, WHITE);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

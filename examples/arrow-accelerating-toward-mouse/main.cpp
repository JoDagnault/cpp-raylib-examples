#include "raylib.h"
#include "raymath.h"
#include <random>

int main() {
  constexpr int screenWidth = 1200;
  constexpr int screenHeight = 800;
  constexpr float accelerationMagnitude = 0.2;
  constexpr float maxSpeed = 5;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> widthDist(0, screenWidth);
  std::uniform_int_distribution<int> heightDist(0, screenHeight);

  Vector2 position =
      Vector2{.x = (float)widthDist(gen), .y = (float)heightDist(gen)};
  Vector2 velocity = Vector2{.x = 0, .y = 0};
  Vector2 acceleration = Vector2{.x = 0, .y = 0};

  InitWindow(screenWidth, screenHeight, "Arrow Accelerating Towards Mouse");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    Vector2 mousePosition = GetMousePosition();

    Vector2 direction = Vector2Subtract(mousePosition, position);
    Vector2 normalizedDirection = Vector2Normalize(direction);

    acceleration = Vector2Scale(normalizedDirection, accelerationMagnitude);
    velocity = Vector2Add(velocity, acceleration);
    velocity = Vector2ClampValue(velocity, 0, maxSpeed);

    position = Vector2Add(position, velocity);

    float angle = atan2f(velocity.y, velocity.x);

    // The original triangle points right to match atan convention that right =
    // 0 degrees
    Vector2 bottomLeftOffset = Vector2Rotate(Vector2{-30, -10}, angle);
    Vector2 bottomRightOffset = Vector2Rotate(Vector2{-30, 10}, angle);

    Vector2 bottomLeft = Vector2Add(position, bottomLeftOffset);
    Vector2 bottomRight = Vector2Add(position, bottomRightOffset);

    ClearBackground(BLACK);
    DrawTriangle(position, bottomLeft, bottomRight, RED);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

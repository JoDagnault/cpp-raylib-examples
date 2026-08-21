#include "raylib.h"
#include "raymath.h"
#include <random>

int main() {
  constexpr int screenWidth = 1200;
  constexpr int screenHeight = 800;
  constexpr int ballRadius = 20;
  constexpr float accelerationMagnitude = 0.2;
  constexpr float maxSpeed = 5;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> widthDist(ballRadius,
                                               screenWidth - ballRadius);
  std::uniform_int_distribution<int> heightDist(ballRadius,
                                                screenHeight - ballRadius);

  Vector2 position =
      Vector2{.x = (float)widthDist(gen), .y = (float)heightDist(gen)};
  Vector2 velocity = Vector2{.x = 0, .y = 0};
  Vector2 acceleration = Vector2{.x = 0, .y = 0};

  InitWindow(screenWidth, screenHeight, "Ball Accelerating Towards Mouse");

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

    ClearBackground(BLACK);
    DrawCircleV(position, ballRadius, RED);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

#include "raylib.h"
#include "raymath.h"
#include <random>

int main() {
  constexpr int SCREEN_WIDTH = 1200;
  constexpr int SCREEN_HEIGHT = 800;
  constexpr float ACCELERATION_MAGNITUDE = 0.2;
  constexpr float MAX_SPEED = 5;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> width_dist(0, SCREEN_WIDTH);
  std::uniform_int_distribution<int> height_dist(0, SCREEN_HEIGHT);

  Vector2 position =
      Vector2{.x = (float)width_dist(gen), .y = (float)height_dist(gen)};
  Vector2 velocity = Vector2{.x = 0, .y = 0};
  Vector2 acceleration = Vector2{.x = 0, .y = 0};

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Arrow Accelerating Towards Mouse");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    Vector2 mouse_position = GetMousePosition();

    Vector2 direction = Vector2Subtract(mouse_position, position);
    Vector2 normalized_direction = Vector2Normalize(direction);

    acceleration = Vector2Scale(normalized_direction, ACCELERATION_MAGNITUDE);
    velocity = Vector2Add(velocity, acceleration);
    velocity = Vector2ClampValue(velocity, 0, MAX_SPEED);

    position = Vector2Add(position, velocity);

    float angle = atan2f(velocity.y, velocity.x);

    // The original triangle points right to match atan convention that right =
    // 0 degrees
    Vector2 bottom_left_offset = Vector2Rotate(Vector2{-30, -10}, angle);
    Vector2 bottom_right_offset = Vector2Rotate(Vector2{-30, 10}, angle);

    Vector2 bottom_left = Vector2Add(position, bottom_left_offset);
    Vector2 bottom_right = Vector2Add(position, bottom_right_offset);

    ClearBackground(BLACK);
    DrawTriangle(position, bottom_left, bottom_right, RED);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

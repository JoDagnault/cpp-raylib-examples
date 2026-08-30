#include "raylib.h"
#include "raymath.h"
#include <random>

int main() {
  constexpr int SCREEN_WIDTH = 1200;
  constexpr int SCREEN_HEIGHT = 800;
  constexpr int BALL_RADIUS = 20;
  constexpr float ACCELERATION_MAGNITUDE = 0.2;
  constexpr float MAX_SPEED = 5;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> width_dist(BALL_RADIUS,
                                                SCREEN_WIDTH - BALL_RADIUS);
  std::uniform_int_distribution<int> height_dist(BALL_RADIUS,
                                                 SCREEN_HEIGHT - BALL_RADIUS);

  Vector2 position =
      Vector2{.x = (float)width_dist(gen), .y = (float)height_dist(gen)};
  Vector2 velocity = Vector2{.x = 0, .y = 0};
  Vector2 acceleration = Vector2{.x = 0, .y = 0};

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ball Accelerating Towards Mouse");

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

    ClearBackground(BLACK);
    DrawCircleV(position, BALL_RADIUS, RED);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

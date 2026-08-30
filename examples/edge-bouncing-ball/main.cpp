#include "raylib.h"
#include "raymath.h"
#include <random>

int main() {
  constexpr int SCREEN_WIDTH = 1200;
  constexpr int SCREEN_HEIGHT = 800;
  constexpr int BALL_RADIUS = 20;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> width_dist(BALL_RADIUS,
                                                SCREEN_WIDTH - BALL_RADIUS);
  std::uniform_int_distribution<int> height_dist(BALL_RADIUS,
                                                 SCREEN_HEIGHT - BALL_RADIUS);
  std::uniform_int_distribution<int> velocity_dist(3, 10);

  Vector2 position =
      Vector2{.x = (float)width_dist(gen), .y = (float)height_dist(gen)};
  Vector2 velocity =
      Vector2{.x = (float)velocity_dist(gen), .y = (float)velocity_dist(gen)};

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Edge Bouncing Ball");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    position = Vector2Add(position, velocity);

    if (position.x - BALL_RADIUS < 0 ||
        position.x + BALL_RADIUS > SCREEN_WIDTH) {
      velocity = Vector2{.x = -velocity.x, .y = velocity.y};
    }
    if (position.y - BALL_RADIUS < 0 ||
        position.y + BALL_RADIUS > SCREEN_HEIGHT) {
      velocity = Vector2{.x = velocity.x, .y = -velocity.y};
    }

    ClearBackground(BLACK);
    DrawCircleV(position, BALL_RADIUS, RED);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

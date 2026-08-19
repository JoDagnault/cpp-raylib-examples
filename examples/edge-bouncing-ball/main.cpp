#include <random>
#include "raylib.h"
#include "raymath.h"

int main() {
  constexpr int screenWidth = 1200;
  constexpr int screenHeight = 800;
  constexpr int ballRadius = 20;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> widthDist(ballRadius,
                                               screenWidth - ballRadius);
  std::uniform_int_distribution<int> heightDist(ballRadius,
                                                screenHeight - ballRadius);
  std::uniform_int_distribution<int> velocityDist(3, 10);

  Vector2 position =
      Vector2{.x = (float)widthDist(gen), .y = (float)heightDist(gen)};
  Vector2 velocity =
      Vector2{.x = (float)velocityDist(gen), .y = (float)velocityDist(gen)};

  InitWindow(screenWidth, screenHeight, "Edge Bouncing Ball");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    position = Vector2Add(position, velocity);

    if (position.x - ballRadius < 0 || position.x + ballRadius > screenWidth) {
      velocity = Vector2{.x = -velocity.x, .y = velocity.y};
    }
    if (position.y - ballRadius < 0 || position.y + ballRadius > screenHeight) {
      velocity = Vector2{.x = velocity.x, .y = -velocity.y};
    }

    ClearBackground(BLACK);
    DrawCircleV(position, ballRadius, RED);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

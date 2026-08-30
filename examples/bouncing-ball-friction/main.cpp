#include "raylib.h"
#include "raymath.h"
#include <random>

int main() {
  constexpr int SCREEN_WIDTH = 1200;
  constexpr int SCREEN_HEIGHT = 800;
  constexpr int BALL_MASS = 10;
  constexpr int BALL_RADIUS = BALL_MASS * 2;
  constexpr Vector2 GRAVITY = Vector2{.x = 0, .y = 1.25};
  constexpr float FRICTION_COEFFICIENT = 0.98;
  constexpr float RESTITUTION_COEFFICIENT = 0.9;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> width_dist(BALL_RADIUS, SCREEN_WIDTH / 2);
  std::uniform_int_distribution<int> height_dist(BALL_RADIUS,
                                                 SCREEN_HEIGHT / 2);

  Vector2 position =
      Vector2{.x = (float)width_dist(gen), .y = (float)height_dist(gen)};
  Vector2 velocity = Vector2{.x = 12, .y = 0};
  Vector2 acceleration = Vector2{.x = 0, .y = 0};

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Boucing Ball Friction");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    acceleration = Vector2Add(acceleration, GRAVITY);
    velocity = Vector2Add(velocity, acceleration);
    position = Vector2Add(position, velocity);
    acceleration = Vector2Scale(acceleration, 0);

    if (position.x - BALL_RADIUS < 0) {
      velocity = Vector2{.x = -velocity.x * RESTITUTION_COEFFICIENT,
                         .y = velocity.y * FRICTION_COEFFICIENT};
      position.x = 0 + BALL_RADIUS;
    }
    if (position.x + BALL_RADIUS > SCREEN_WIDTH) {
      velocity = Vector2{.x = -velocity.x * RESTITUTION_COEFFICIENT,
                         .y = velocity.y * FRICTION_COEFFICIENT};
      position.x = SCREEN_WIDTH - BALL_RADIUS;
    }
    if (position.y - BALL_RADIUS < 0) {
      velocity = Vector2{.x = velocity.x * FRICTION_COEFFICIENT,
                         .y = -velocity.y * RESTITUTION_COEFFICIENT};
      position.y = 0 + BALL_RADIUS;
    }
    if (position.y + BALL_RADIUS > SCREEN_HEIGHT) {
      velocity = Vector2{.x = velocity.x * FRICTION_COEFFICIENT,
                         .y = -velocity.y * RESTITUTION_COEFFICIENT};
      position.y = SCREEN_HEIGHT - BALL_RADIUS;
    }

    ClearBackground(BLACK);
    DrawCircleV(position, BALL_RADIUS, RED);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

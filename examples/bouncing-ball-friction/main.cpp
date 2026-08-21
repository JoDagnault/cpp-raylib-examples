#include "raylib.h"
#include "raymath.h"
#include <random>

int main() {
  constexpr int screenWidth = 1200;
  constexpr int screenHeight = 800;
  constexpr int ballMass = 10;
  constexpr int ballRadius = ballMass * 2;
  constexpr Vector2 gravity = Vector2{.x = 0, .y = 1.25};
  constexpr float frictionCoefficient = 0.98;
  constexpr float restitutionCoefficient = 0.9;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> widthDist(ballRadius, screenWidth / 2);
  std::uniform_int_distribution<int> heightDist(ballRadius, screenHeight / 2);

  Vector2 position =
      Vector2{.x = (float)widthDist(gen), .y = (float)heightDist(gen)};
  Vector2 velocity = Vector2{.x = 12, .y = 0};
  Vector2 acceleration = Vector2{.x = 0, .y = 0};

  InitWindow(screenWidth, screenHeight, "Boucing Ball Friction");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    acceleration = Vector2Add(acceleration, gravity);
    velocity = Vector2Add(velocity, acceleration);
    position = Vector2Add(position, velocity);
    acceleration = Vector2Scale(acceleration, 0);

    if (position.x - ballRadius < 0) {
      velocity = Vector2{.x = -velocity.x * restitutionCoefficient,
                         .y = velocity.y * frictionCoefficient};
      position.x = 0 + ballRadius;
    }
    if (position.x + ballRadius > screenWidth) {
      velocity = Vector2{.x = -velocity.x * restitutionCoefficient,
                         .y = velocity.y * frictionCoefficient};
      position.x = screenWidth - ballRadius;
    }
    if (position.y - ballRadius < 0) {
      velocity = Vector2{.x = velocity.x * frictionCoefficient,
                         .y = -velocity.y * restitutionCoefficient};
      position.y = 0 + ballRadius;
    }
    if (position.y + ballRadius > screenHeight) {
      velocity = Vector2{.x = velocity.x * frictionCoefficient,
                         .y = -velocity.y * restitutionCoefficient};
      position.y = screenHeight - ballRadius;
    }

    ClearBackground(BLACK);
    DrawCircleV(position, ballRadius, RED);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

#include "raylib.h"
#include "raymath.h"

int main() {
  constexpr int SCREEN_WIDTH = 1200;
  constexpr int SCREEN_HEIGHT = 800;
  constexpr Vector2 CENTER =
      Vector2{.x = SCREEN_WIDTH / 2, .y = SCREEN_HEIGHT / 2};
  constexpr int BALL_RADIUS = 20;
  constexpr int STICK_LENGTH = 200;
  constexpr int STICK_THICKNESS = 3;
  constexpr float ANGULAR_ACCELERATION = 0.01 * DEG2RAD;
  constexpr float MAX_ANGULAR_VELOCITY = 10 * DEG2RAD;
  float angular_velocity = 0;
  float angle = 0;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rotating Stick");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    angular_velocity += ANGULAR_ACCELERATION;
    if (angular_velocity > MAX_ANGULAR_VELOCITY) {
      angular_velocity = MAX_ANGULAR_VELOCITY;
    }
    angle += angular_velocity;

    Vector2 offset = Vector2Rotate(Vector2{STICK_LENGTH / 2, 0}, angle);

    Vector2 start = Vector2Subtract(CENTER, offset);
    Vector2 end = Vector2Add(CENTER, offset);

    DrawLineEx(start, end, STICK_THICKNESS, WHITE);
    DrawCircleV(start, BALL_RADIUS, WHITE);
    DrawCircleV(end, BALL_RADIUS, WHITE);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

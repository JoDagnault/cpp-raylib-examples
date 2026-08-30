#include "raylib.h"
#include "raymath.h"
#include <random>

struct SteeringOutput {
  Vector2 desired_velocity = Vector2{0, 0};
  Vector2 steering_force = Vector2{0, 0};
};

struct Vehicle {
  float mass;
  Vector2 position;
  Vector2 velocity;
  Vector2 acceleration;
  float max_force;
  float max_speed;
  float radius;
  Vector2 forward;
  Vector2 right;

  void applyForce(Vector2 force) {
    acceleration = Vector2Add(acceleration, Vector2Scale(force, 1.0f / mass));
  }

  SteeringOutput seek(Vector2 target_position) {
    SteeringOutput output;
    Vector2 offset = Vector2Subtract(target_position, position);

    if (Vector2Length(offset) > radius) {
      return output;
    }

    output.desired_velocity = Vector2Normalize(offset) * max_speed;

    output.steering_force = Vector2ClampValue(
        Vector2Subtract(output.desired_velocity, velocity), 0.0f, max_force);

    applyForce(output.steering_force);
    return output;
  }

  SteeringOutput flee(Vector2 target_position) {
    SteeringOutput output;
    Vector2 offset = Vector2Subtract(position, target_position);

    if (Vector2Length(offset) > radius) {
      return output;
    }

    output.desired_velocity = Vector2Normalize(offset) * max_speed;

    output.steering_force = Vector2ClampValue(
        Vector2Subtract(output.desired_velocity, velocity), 0.0f, max_force);

    applyForce(output.steering_force);
    return output;
  }

  void update() {
    velocity =
        Vector2ClampValue(Vector2Add(velocity, acceleration), 0, max_speed);

    if (Vector2LengthSqr(velocity) > 0.0f) {
      forward = Vector2Normalize(velocity);
      right = Vector2{.x = -forward.y, .y = forward.x};
    }

    position = Vector2Add(position, velocity);
    acceleration = Vector2{0, 0};
  }

  void wrapEdges(float screen_width, float screen_height) {
    if (position.x < 0) {
      position.x += screen_width;
    } else if (position.x >= screen_width) {
      position.x -= screen_width;
    }

    if (position.y < 0) {
      position.y += screen_height;
    } else if (position.y >= screen_height) {
      position.y -= screen_height;
    }
  }
};

class VehicleRenderer {
public:
  VehicleRenderer(float length, float half_width)
      : length(length), half_width(half_width) {}

  void render(const Vehicle &vehicle) const {
    Vector2 arrow_tip = Vector2Add(
        vehicle.position, Vector2Scale(vehicle.forward, length * 2.0f / 3.0f));

    Vector2 arrow_base_center = Vector2Subtract(
        vehicle.position, Vector2Scale(vehicle.forward, length * 1.0f / 3.0f));

    Vector2 arrow_bottom_left = Vector2Subtract(
        arrow_base_center, Vector2Scale(vehicle.right, half_width));

    Vector2 arrow_bottom_right =
        Vector2Add(arrow_base_center, Vector2Scale(vehicle.right, half_width));

    DrawTriangle(arrow_tip, arrow_bottom_left, arrow_bottom_right, RED);
  }

  void renderDebug(const Vehicle &vehicle,
                   const SteeringOutput &steering) const {
    constexpr float VELOCITY_SCALE = 12.0f;
    constexpr float STEERING_SCALE = 30.0f;
    constexpr float LINE_THICKNESS = 3.0f;

    DrawLineEx(vehicle.position,
               Vector2Add(vehicle.position,
                          Vector2Scale(vehicle.velocity, VELOCITY_SCALE)),
               LINE_THICKNESS, SKYBLUE);
    DrawLineEx(
        vehicle.position,
        Vector2Add(vehicle.position,
                   Vector2Scale(steering.desired_velocity, VELOCITY_SCALE)),
        LINE_THICKNESS, LIME);
    DrawLineEx(
        vehicle.position,
        Vector2Add(vehicle.position,
                   Vector2Scale(steering.steering_force, STEERING_SCALE)),
        LINE_THICKNESS, ORANGE);

    constexpr int INFO_X = 20;
    constexpr int INFO_Y = 80;
    constexpr int FONT_SIZE = 18;
    constexpr int LINE_SPACING = 24;

    DrawText(TextFormat("Velocity: (%.2f, %.2f)", vehicle.velocity.x,
                        vehicle.velocity.y),
             INFO_X, INFO_Y, FONT_SIZE, SKYBLUE);
    DrawText(TextFormat("Desired: (%.2f, %.2f)", steering.desired_velocity.x,
                        steering.desired_velocity.y),
             INFO_X, INFO_Y + LINE_SPACING, FONT_SIZE, LIME);
    DrawText(TextFormat("Steering: (%.2f, %.2f)", steering.steering_force.x,
                        steering.steering_force.y),
             INFO_X, INFO_Y + LINE_SPACING * 2, FONT_SIZE, ORANGE);
  }

private:
  float length;
  float half_width;
};

int main() {
  constexpr int SCREEN_WIDTH = 1200;
  constexpr int SCREEN_HEIGHT = 800;
  constexpr float MASS = 10;
  constexpr float MAX_FORCE = 2;
  constexpr float MAX_SPEED = 5;
  constexpr float ARROW_LENGTH = 30;
  constexpr float ARROW_HALF_WIDTH = 10;
  constexpr int TARGET_RADIUS = 20;
  constexpr float BEHAVIOR_RADIUS = 300;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> width_dist(0, SCREEN_WIDTH);
  std::uniform_int_distribution<int> height_dist(0, SCREEN_HEIGHT);
  std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * PI);

  Vector2 position =
      Vector2{.x = (float)width_dist(gen), .y = (float)height_dist(gen)};
  Vector2 velocity = Vector2Rotate(Vector2{MAX_SPEED, 0}, angle_dist(gen));
  Vector2 forward = Vector2Normalize(velocity);
  Vector2 right = Vector2{.x = -forward.y, .y = forward.x};

  Vehicle vehicle = Vehicle{.mass = MASS,
                            .position = position,
                            .velocity = velocity,
                            .acceleration = Vector2{0, 0},
                            .max_force = MAX_FORCE,
                            .max_speed = MAX_SPEED,
                            .radius = BEHAVIOR_RADIUS,
                            .forward = forward,
                            .right = right};

  VehicleRenderer renderer(ARROW_LENGTH, ARROW_HALF_WIDTH);

  Rectangle behavior_button = Rectangle{20, 20, 160, 40};
  bool flee_mode = true;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Seek And Flee");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    Vector2 mouse_position = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mouse_position, behavior_button)) {
      flee_mode = !flee_mode;
    }

    SteeringOutput steering =
        flee_mode ? vehicle.flee(mouse_position) : vehicle.seek(mouse_position);

    vehicle.update();
    vehicle.wrapEdges(SCREEN_WIDTH, SCREEN_HEIGHT);

    ClearBackground(BLACK);

    DrawCircleV(mouse_position, TARGET_RADIUS, GREEN);
    renderer.renderDebug(vehicle, steering);
    renderer.render(vehicle);

    bool button_hovered =
        CheckCollisionPointRec(mouse_position, behavior_button);
    DrawRectangleRec(behavior_button, button_hovered ? GRAY : DARKGRAY);
    DrawRectangleLinesEx(behavior_button, 2, LIGHTGRAY);

    const char *button_text = flee_mode ? "Mode: Flee" : "Mode: Seek";
    constexpr int BUTTON_FONT_SIZE = 20;
    int button_text_width = MeasureText(button_text, BUTTON_FONT_SIZE);
    DrawText(
        button_text,
        behavior_button.x + (behavior_button.width - button_text_width) / 2.0f,
        behavior_button.y + (behavior_button.height - BUTTON_FONT_SIZE) / 2.0f,
        BUTTON_FONT_SIZE, WHITE);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

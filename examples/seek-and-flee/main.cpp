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

  void apply_force(Vector2 force) {
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

    apply_force(output.steering_force);
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

    apply_force(output.steering_force);
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

  void wrap_edges(float screen_width, float screen_height) {
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

  void render_debug(const Vehicle &vehicle,
                    const SteeringOutput &steering) const {
    constexpr float velocity_scale = 12.0f;
    constexpr float steering_scale = 30.0f;
    constexpr float line_thickness = 3.0f;

    DrawLineEx(vehicle.position,
               Vector2Add(vehicle.position,
                          Vector2Scale(vehicle.velocity, velocity_scale)),
               line_thickness, SKYBLUE);
    DrawLineEx(
        vehicle.position,
        Vector2Add(vehicle.position,
                   Vector2Scale(steering.desired_velocity, velocity_scale)),
        line_thickness, LIME);
    DrawLineEx(
        vehicle.position,
        Vector2Add(vehicle.position,
                   Vector2Scale(steering.steering_force, steering_scale)),
        line_thickness, ORANGE);

    constexpr int info_x = 20;
    constexpr int info_y = 80;
    constexpr int font_size = 18;
    constexpr int line_spacing = 24;

    DrawText(TextFormat("Velocity: (%.2f, %.2f)", vehicle.velocity.x,
                        vehicle.velocity.y),
             info_x, info_y, font_size, SKYBLUE);
    DrawText(TextFormat("Desired: (%.2f, %.2f)", steering.desired_velocity.x,
                        steering.desired_velocity.y),
             info_x, info_y + line_spacing, font_size, LIME);
    DrawText(TextFormat("Steering: (%.2f, %.2f)", steering.steering_force.x,
                        steering.steering_force.y),
             info_x, info_y + line_spacing * 2, font_size, ORANGE);
  }

private:
  float length;
  float half_width;
};

int main() {
  constexpr int screen_width = 1200;
  constexpr int screen_height = 800;
  constexpr float mass = 10;
  constexpr float max_force = 2;
  constexpr float max_speed = 5;
  constexpr float arrow_length = 30;
  constexpr float arrow_half_width = 10;
  constexpr int target_radius = 20;
  constexpr float behavior_radius = 300;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> width_dist(0, screen_width);
  std::uniform_int_distribution<int> height_dist(0, screen_height);
  std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * PI);

  Vector2 position =
      Vector2{.x = (float)width_dist(gen), .y = (float)height_dist(gen)};
  Vector2 velocity = Vector2Rotate(Vector2{max_speed, 0}, angle_dist(gen));
  Vector2 forward = Vector2Normalize(velocity);
  Vector2 right = Vector2{.x = -forward.y, .y = forward.x};

  Vehicle vehicle = Vehicle{.mass = mass,
                            .position = position,
                            .velocity = velocity,
                            .acceleration = Vector2{0, 0},
                            .max_force = max_force,
                            .max_speed = max_speed,
                            .radius = behavior_radius,
                            .forward = forward,
                            .right = right};

  VehicleRenderer renderer(arrow_length, arrow_half_width);

  Rectangle behavior_button = Rectangle{20, 20, 160, 40};
  bool flee_mode = true;

  InitWindow(screen_width, screen_height, "Seek And Flee");

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
    vehicle.wrap_edges(screen_width, screen_height);

    ClearBackground(BLACK);

    DrawCircleV(mouse_position, target_radius, GREEN);
    renderer.render_debug(vehicle, steering);
    renderer.render(vehicle);

    bool button_hovered =
        CheckCollisionPointRec(mouse_position, behavior_button);
    DrawRectangleRec(behavior_button, button_hovered ? GRAY : DARKGRAY);
    DrawRectangleLinesEx(behavior_button, 2, LIGHTGRAY);

    const char *button_text = flee_mode ? "Mode: Flee" : "Mode: Seek";
    constexpr int button_font_size = 20;
    int button_text_width = MeasureText(button_text, button_font_size);
    DrawText(
        button_text,
        behavior_button.x + (behavior_button.width - button_text_width) / 2.0f,
        behavior_button.y + (behavior_button.height - button_font_size) / 2.0f,
        button_font_size, WHITE);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

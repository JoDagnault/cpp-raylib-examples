#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <random>
#include <vector>

struct Obstacle {
  Vector2 position;
  float radius;
};

struct SteeringOutput {
  Vector2 desired_velocity = Vector2{0, 0};
  Vector2 steering_force = Vector2{0, 0};
  float detection_length = 0.0f;
};

struct Vehicle {
  float mass;
  Vector2 position;
  Vector2 velocity;
  Vector2 acceleration;
  float max_force;
  float max_speed;
  float radius;
  float min_detection_length;
  Vector2 forward;
  Vector2 right;

  void applyForce(Vector2 force) {
    acceleration = Vector2Add(acceleration, Vector2Scale(force, 1.0f / mass));
  }

  SteeringOutput avoidObstacles(const std::vector<Obstacle> &obstacles) {
    SteeringOutput output;
    output.detection_length =
        min_detection_length * (1.0f + Vector2Length(velocity) / max_speed);

    bool found = false;
    float nearest_ahead = 0.0f;
    float nearest_lateral = 0.0f;
    float nearest_clearance = 0.0f;

    for (const Obstacle &obstacle : obstacles) {
      Vector2 offset = Vector2Subtract(obstacle.position, position);
      float ahead = Vector2DotProduct(offset, forward);
      float lateral = Vector2DotProduct(offset, right);
      float clearance = obstacle.radius + radius;

      bool in_box = ahead + obstacle.radius > 0.0f &&
                    ahead - obstacle.radius < output.detection_length &&
                    fabsf(lateral) < clearance;

      if (in_box && (!found || ahead < nearest_ahead)) {
        found = true;
        nearest_ahead = ahead;
        nearest_lateral = lateral;
        nearest_clearance = clearance;
      }
    }

    if (!found) {
      return output;
    }

    float intrusion = nearest_clearance - fabsf(nearest_lateral);
    Vector2 away = Vector2Scale(right, nearest_lateral > 0.0f ? -1.0f : 1.0f);

    output.desired_velocity =
        Vector2Normalize(Vector2Add(Vector2Scale(forward, max_speed),
                                    Vector2Scale(away, intrusion))) *
        max_speed;

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

    Rectangle detection_box =
        Rectangle{vehicle.position.x, vehicle.position.y,
                  steering.detection_length, vehicle.radius * 2.0f};
    float heading = atan2f(vehicle.forward.y, vehicle.forward.x) * RAD2DEG;
    DrawRectanglePro(detection_box, Vector2{0, vehicle.radius}, heading,
                     Fade(DARKGRAY, 0.5f));
    DrawCircleLinesV(vehicle.position, vehicle.radius, DARKGRAY);

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
    constexpr int INFO_Y = 20;
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
  constexpr float VEHICLE_RADIUS = 15;
  constexpr float MIN_DETECTION_LENGTH = 100;
  constexpr float ARROW_LENGTH = 30;
  constexpr float ARROW_HALF_WIDTH = 10;

  std::vector<Obstacle> obstacles = {
      Obstacle{.position = Vector2{200, 200}, .radius = 50},
      Obstacle{.position = Vector2{600, 180}, .radius = 40},
      Obstacle{.position = Vector2{1000, 220}, .radius = 60},
      Obstacle{.position = Vector2{350, 500}, .radius = 60},
      Obstacle{.position = Vector2{780, 430}, .radius = 45},
      Obstacle{.position = Vector2{1050, 620}, .radius = 35},
      Obstacle{.position = Vector2{550, 720}, .radius = 40}};

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
                            .radius = VEHICLE_RADIUS,
                            .min_detection_length = MIN_DETECTION_LENGTH,
                            .forward = forward,
                            .right = right};

  VehicleRenderer renderer(ARROW_LENGTH, ARROW_HALF_WIDTH);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Obstacle Avoidance");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    SteeringOutput steering = vehicle.avoidObstacles(obstacles);

    vehicle.update();
    vehicle.wrapEdges(SCREEN_WIDTH, SCREEN_HEIGHT);

    ClearBackground(BLACK);

    for (const Obstacle &obstacle : obstacles) {
      DrawCircleV(obstacle.position, obstacle.radius, GRAY);
    }
    renderer.renderDebug(vehicle, steering);
    renderer.render(vehicle);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

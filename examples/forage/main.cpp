#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <random>
#include <vector>

struct SteeringOutput {
  Vector2 desired_velocity = Vector2{0, 0};
  Vector2 steering_force = Vector2{0, 0};
  Vector2 wander_center = Vector2{0, 0};
  Vector2 wander_target = Vector2{0, 0};
  bool seeking_food = false;
};

struct Vehicle {
  float mass;
  Vector2 position;
  Vector2 velocity;
  Vector2 acceleration;
  float max_force;
  float max_speed;
  float wander_distance;
  float wander_radius;
  float wander_jitter;
  float wander_angle;
  float sense_radius;
  float hunger_rate;
  float hunger_threshold;
  float hunger;
  Vector2 forward;
  Vector2 right;

  void applyForce(Vector2 force) {
    acceleration = Vector2Add(acceleration, Vector2Scale(force, 1.0f / mass));
  }

  SteeringOutput wander(std::mt19937 &gen) {
    SteeringOutput output;
    std::uniform_real_distribution<float> jitter_dist(-wander_jitter,
                                                      wander_jitter);
    wander_angle += jitter_dist(gen);

    output.wander_center =
        Vector2Add(position, Vector2Scale(forward, wander_distance));
    output.wander_target =
        Vector2Add(output.wander_center,
                   Vector2Scale(Vector2{cosf(wander_angle), sinf(wander_angle)},
                                wander_radius));

    Vector2 offset = Vector2Subtract(output.wander_target, position);
    output.desired_velocity = Vector2Normalize(offset) * max_speed;

    output.steering_force = Vector2ClampValue(
        Vector2Subtract(output.desired_velocity, velocity), 0.0f, max_force);

    applyForce(output.steering_force);
    return output;
  }

  SteeringOutput seek(Vector2 target_position) {
    SteeringOutput output;
    Vector2 offset = Vector2Subtract(target_position, position);
    output.desired_velocity = Vector2Normalize(offset) * max_speed;

    output.steering_force = Vector2ClampValue(
        Vector2Subtract(output.desired_velocity, velocity), 0.0f, max_force);

    applyForce(output.steering_force);
    return output;
  }

  SteeringOutput forage(const std::vector<Vector2> &food, std::mt19937 &gen) {
    hunger = fminf(hunger + hunger_rate, 1.0f);

    if (hunger < hunger_threshold) {
      return wander(gen);
    }

    bool found = false;
    Vector2 nearest_food = Vector2{0, 0};
    float nearest_distance = sense_radius;

    for (const Vector2 &food_position : food) {
      float distance = Vector2Distance(position, food_position);
      if (distance < nearest_distance) {
        found = true;
        nearest_food = food_position;
        nearest_distance = distance;
      }
    }

    if (!found) {
      return wander(gen);
    }

    SteeringOutput output = seek(nearest_food);
    output.seeking_food = true;
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
    constexpr float TARGET_RADIUS = 6.0f;

    DrawCircleLinesV(vehicle.position, vehicle.sense_radius, DARKGRAY);
    if (!steering.seeking_food) {
      DrawCircleLinesV(steering.wander_center, vehicle.wander_radius, DARKGRAY);
      DrawCircleV(steering.wander_target, TARGET_RADIUS, GREEN);
    }

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
    DrawText(steering.seeking_food ? "Mode: Seek food" : "Mode: Wander", INFO_X,
             INFO_Y + LINE_SPACING * 3, FONT_SIZE, WHITE);

    constexpr int BAR_WIDTH = 200;
    constexpr int BAR_HEIGHT = 12;
    int bar_y = INFO_Y + LINE_SPACING * 4;
    bool hungry = vehicle.hunger >= vehicle.hunger_threshold;
    int threshold_x = INFO_X + (int)(BAR_WIDTH * vehicle.hunger_threshold);

    DrawRectangle(INFO_X, bar_y, (int)(BAR_WIDTH * vehicle.hunger), BAR_HEIGHT,
                  hungry ? RED : GREEN);
    DrawRectangleLines(INFO_X, bar_y, BAR_WIDTH, BAR_HEIGHT, LIGHTGRAY);
    DrawLine(threshold_x, bar_y, threshold_x, bar_y + BAR_HEIGHT, WHITE);
    DrawText("Hunger", INFO_X + BAR_WIDTH + 10, bar_y - 3, FONT_SIZE,
             LIGHTGRAY);
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
  constexpr float WANDER_DISTANCE = 120;
  constexpr float WANDER_RADIUS = 60;
  constexpr float WANDER_JITTER = 0.2f;
  constexpr float SENSE_RADIUS = 150;
  constexpr float HUNGER_RATE = 1.0f / 600;
  constexpr float HUNGER_THRESHOLD = 0.5f;
  constexpr int FOOD_COUNT = 8;
  constexpr float FOOD_RADIUS = 8;
  constexpr float ARROW_LENGTH = 30;
  constexpr float ARROW_HALF_WIDTH = 10;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> width_dist(0, SCREEN_WIDTH);
  std::uniform_int_distribution<int> height_dist(0, SCREEN_HEIGHT);
  std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * PI);

  auto random_position = [&]() {
    return Vector2{.x = (float)width_dist(gen), .y = (float)height_dist(gen)};
  };

  std::vector<Vector2> food(FOOD_COUNT);
  for (Vector2 &food_position : food) {
    food_position = random_position();
  }

  Vector2 position = random_position();
  float heading = angle_dist(gen);
  Vector2 velocity = Vector2Rotate(Vector2{MAX_SPEED, 0}, heading);
  Vector2 forward = Vector2Normalize(velocity);
  Vector2 right = Vector2{.x = -forward.y, .y = forward.x};

  Vehicle vehicle = Vehicle{.mass = MASS,
                            .position = position,
                            .velocity = velocity,
                            .acceleration = Vector2{0, 0},
                            .max_force = MAX_FORCE,
                            .max_speed = MAX_SPEED,
                            .wander_distance = WANDER_DISTANCE,
                            .wander_radius = WANDER_RADIUS,
                            .wander_jitter = WANDER_JITTER,
                            .wander_angle = heading,
                            .sense_radius = SENSE_RADIUS,
                            .hunger_rate = HUNGER_RATE,
                            .hunger_threshold = HUNGER_THRESHOLD,
                            .hunger = 0.0f,
                            .forward = forward,
                            .right = right};

  VehicleRenderer renderer(ARROW_LENGTH, ARROW_HALF_WIDTH);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Forage");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    SteeringOutput steering = vehicle.forage(food, gen);

    vehicle.update();
    vehicle.wrapEdges(SCREEN_WIDTH, SCREEN_HEIGHT);

    if (steering.seeking_food) {
      for (Vector2 &food_position : food) {
        if (Vector2Distance(vehicle.position, food_position) < FOOD_RADIUS) {
          vehicle.hunger = 0.0f;
          food_position = random_position();
        }
      }
    }

    ClearBackground(BLACK);

    for (const Vector2 &food_position : food) {
      DrawCircleV(food_position, FOOD_RADIUS, YELLOW);
    }
    renderer.renderDebug(vehicle, steering);
    renderer.render(vehicle);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

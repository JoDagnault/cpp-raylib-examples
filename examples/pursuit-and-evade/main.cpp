#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <random>

Vector2 wrapPosition(Vector2 position, Vector2 world_size) {
  position.x = fmodf(position.x, world_size.x);
  position.y = fmodf(position.y, world_size.y);

  if (position.x < 0) {
    position.x += world_size.x;
  }
  if (position.y < 0) {
    position.y += world_size.y;
  }

  return position;
}

Vector2 wrappedOffset(Vector2 from, Vector2 to, Vector2 world_size) {
  Vector2 half = Vector2Scale(world_size, 0.5f);
  Vector2 offset = Vector2Subtract(to, from);
  return Vector2Subtract(wrapPosition(Vector2Add(offset, half), world_size),
                         half);
}

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
  Vector2 world_size;
  Vector2 forward;
  Vector2 right;

  void applyForce(Vector2 force) {
    acceleration = Vector2Add(acceleration, Vector2Scale(force, 1.0f / mass));
  }

  Vector2 predictPosition(const Vehicle &other) const {
    Vector2 offset = wrappedOffset(position, other.position, world_size);
    float distance = Vector2Length(offset);
    float prediction = distance / (max_speed + Vector2Length(other.velocity));

    Vector2 nearest_image = Vector2Add(position, offset);
    return Vector2Add(nearest_image, Vector2Scale(other.velocity, prediction));
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

  SteeringOutput flee(Vector2 target_position) {
    SteeringOutput output;
    Vector2 offset = Vector2Subtract(position, target_position);

    output.desired_velocity = Vector2Normalize(offset) * max_speed;

    output.steering_force = Vector2ClampValue(
        Vector2Subtract(output.desired_velocity, velocity), 0.0f, max_force);

    applyForce(output.steering_force);
    return output;
  }

  SteeringOutput pursue(const Vehicle &quarry) {
    return seek(predictPosition(quarry));
  }

  SteeringOutput evade(const Vehicle &pursuer) {
    return flee(predictPosition(pursuer));
  }

  void update() {
    velocity =
        Vector2ClampValue(Vector2Add(velocity, acceleration), 0, max_speed);

    if (Vector2LengthSqr(velocity) > 0.0f) {
      forward = Vector2Normalize(velocity);
      right = Vector2{.x = -forward.y, .y = forward.x};
    }

    position = wrapPosition(Vector2Add(position, velocity), world_size);
    acceleration = Vector2{0, 0};
  }
};

void respawn(Vehicle &vehicle, std::mt19937 &gen) {
  std::uniform_real_distribution<float> x_dist(0.0f, vehicle.world_size.x);
  std::uniform_real_distribution<float> y_dist(0.0f, vehicle.world_size.y);
  std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * PI);

  vehicle.position = Vector2{.x = x_dist(gen), .y = y_dist(gen)};
  vehicle.velocity =
      Vector2Rotate(Vector2{vehicle.max_speed, 0}, angle_dist(gen));
  vehicle.acceleration = Vector2{0, 0};
  vehicle.forward = Vector2Normalize(vehicle.velocity);
  vehicle.right = Vector2{.x = -vehicle.forward.y, .y = vehicle.forward.x};
}

void respawnApart(Vehicle &first, Vehicle &second, std::mt19937 &gen,
                  float min_distance) {
  do {
    respawn(first, gen);
    respawn(second, gen);
  } while (Vector2Length(wrappedOffset(first.position, second.position,
                                       first.world_size)) < min_distance);
}

class VehicleRenderer {
public:
  VehicleRenderer(float length, float half_width)
      : length(length), half_width(half_width) {}

  void render(const Vehicle &vehicle, Color color) const {
    Vector2 arrow_tip = Vector2Add(
        vehicle.position, Vector2Scale(vehicle.forward, length * 2.0f / 3.0f));

    Vector2 arrow_base_center = Vector2Subtract(
        vehicle.position, Vector2Scale(vehicle.forward, length * 1.0f / 3.0f));

    Vector2 arrow_bottom_left = Vector2Subtract(
        arrow_base_center, Vector2Scale(vehicle.right, half_width));

    Vector2 arrow_bottom_right =
        Vector2Add(arrow_base_center, Vector2Scale(vehicle.right, half_width));

    DrawTriangle(arrow_tip, arrow_bottom_left, arrow_bottom_right, color);
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
  }

private:
  float length;
  float half_width;
};

int main() {
  constexpr int SCREEN_WIDTH = 1200;
  constexpr int SCREEN_HEIGHT = 800;
  constexpr Vector2 WORLD_SIZE = Vector2{SCREEN_WIDTH, SCREEN_HEIGHT};
  constexpr float MASS = 10;
  constexpr float MAX_FORCE = 1;
  constexpr float PURSUER_MAX_SPEED = 7;
  constexpr float EVADER_MAX_SPEED = 5;
  constexpr float ARROW_LENGTH = 30;
  constexpr float ARROW_HALF_WIDTH = 10;
  constexpr float CATCH_RADIUS = 15;
  constexpr float MIN_SPAWN_DISTANCE = 300;

  std::random_device rd;
  std::mt19937 gen(rd());

  Vehicle pursuer = Vehicle{.mass = MASS,
                            .position = Vector2{0, 0},
                            .velocity = Vector2{0, 0},
                            .acceleration = Vector2{0, 0},
                            .max_force = MAX_FORCE,
                            .max_speed = PURSUER_MAX_SPEED,
                            .world_size = WORLD_SIZE,
                            .forward = Vector2{1, 0},
                            .right = Vector2{0, 1}};

  Vehicle evader = Vehicle{.mass = MASS,
                           .position = Vector2{0, 0},
                           .velocity = Vector2{0, 0},
                           .acceleration = Vector2{0, 0},
                           .max_force = MAX_FORCE,
                           .max_speed = EVADER_MAX_SPEED,
                           .world_size = WORLD_SIZE,
                           .forward = Vector2{1, 0},
                           .right = Vector2{0, 1}};

  respawnApart(pursuer, evader, gen, MIN_SPAWN_DISTANCE);

  VehicleRenderer renderer(ARROW_LENGTH, ARROW_HALF_WIDTH);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pursuit And Evade");

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    SteeringOutput pursuer_steering = pursuer.pursue(evader);
    SteeringOutput evader_steering = evader.evade(pursuer);

    pursuer.update();
    evader.update();

    float distance = Vector2Length(
        wrappedOffset(pursuer.position, evader.position, WORLD_SIZE));

    if (distance < CATCH_RADIUS) {
      respawnApart(pursuer, evader, gen, MIN_SPAWN_DISTANCE);
    }

    ClearBackground(BLACK);

    renderer.renderDebug(pursuer, pursuer_steering);
    renderer.renderDebug(evader, evader_steering);
    renderer.render(pursuer, RED);
    renderer.render(evader, GREEN);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

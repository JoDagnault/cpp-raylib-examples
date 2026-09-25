#include "raylib.h"

int main() {
  constexpr int SCREEN_WIDTH = 1200;
  constexpr int SCREEN_HEIGHT = 800;
  constexpr int TERRAIN_RESOLUTION = 64;
  constexpr float TERRAIN_NOISE_SCALE = 0.8f;
  constexpr float CAMERA_FOV_Y = 45.0f;
  constexpr Vector3 TERRAIN_SIZE =
      Vector3{.x = 24.0f, .y = 10.0f, .z = 24.0f};
  constexpr Vector3 TERRAIN_POSITION =
      Vector3{.x = -12.0f, .y = -5.0f, .z = -12.0f};

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Terrain");

  Camera camera = {
      .position = Vector3{.x = 20.0f, .y = 18.0f, .z = 20.0f},
      .target = Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f},
      .up = Vector3{.x = 0.0f, .y = 1.0f, .z = 0.0f},
      .fovy = CAMERA_FOV_Y,
      .projection = CAMERA_PERSPECTIVE,
  };

  Image heightmap =
      GenImagePerlinNoise(TERRAIN_RESOLUTION, TERRAIN_RESOLUTION, 0, 0,
                          TERRAIN_NOISE_SCALE);
  Model terrain = LoadModelFromMesh(GenMeshHeightmap(heightmap, TERRAIN_SIZE));
  UnloadImage(heightmap);

  DisableCursor();
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    UpdateCamera(&camera, CAMERA_THIRD_PERSON);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
    DrawModel(terrain, TERRAIN_POSITION, 1.0f, GREEN);
    DrawModelWires(terrain, TERRAIN_POSITION, 1.0f, DARKGREEN);
    EndMode3D();

    EndDrawing();
  }

  UnloadModel(terrain);
  CloseWindow();

  return 0;
}

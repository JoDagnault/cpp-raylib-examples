# C++ raylib examples

A collection of small C++ toy projects built with raylib.

## Examples

- `edge-bouncing-ball`: simulates a ball bouncing off the edges of the
  window
- `ball-accelerating-toward-mouse`: simulates a ball accelerating towards the mouse position
- `bouncing-ball-friction`: simulates a bouncing ball with friction and restitution coefficients
- `rotating-stick`: simulates a stick spinning faster over time 
- `arrow-accelerating-toward-mouse`: simulates an arrow accelerating towards the mouse and pointing in the velocity direction

## Build and run

```sh
cmake -B build
cmake --build build --target edge-bouncing-ball
./build/edge-bouncing-ball
```

Each example is an independent CMake target, so only the selected example is
built. 

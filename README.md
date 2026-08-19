# C++ raylib examples

A collection of small C++ toy projects built with raylib.

## Examples

- `edge-bouncing-ball` — simulates a ball bouncing off the edges of the
  window.

## Build and run

```sh
cmake -B build
cmake --build build --target edge-bouncing-ball
./build/edge-bouncing-ball
```

Each example is an independent CMake target, so only the selected example is
built. 

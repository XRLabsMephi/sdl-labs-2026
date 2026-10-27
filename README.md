# Software Graphics Labs - Raycaster

A simple software raycaster written in C++20 using SDL3.

The project is developed as part of laboratory assignments at MEPhI.

## Lab 1

The goal is to render a 3D-looking scene from a 2D grid map.

Main features:
- Player movement and rotation
- DDA-based raycasting
- Perspective wall projection
- CPU framebuffer
- SDL3 window and image presentation

## Requirements

- C++20 compiler
- CMake 3.24 or newer
- SDL3 development libraries

## Build

Configure the project in Debug mode:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

Compile all targets:

```bash
cmake --build build
```

## Tests

Run framebuffer tests:

```bash
ctest --test-dir build --output-on-failure
```

## Rendering demo

Run the standalone SDL3 demo:

```bash
./build/wolf_render_demo
```

The demo displays a simple scene with a ceiling,
floor and colored wall columns.

Press Escape or close the window to exit.

The demo does not use DDA or player movement.

## Project structure

- `include/wolf/` - public interfaces
- `src/wolf/` - implementation
- `docs/wolf/` - module documentation
- `tests/` - framebuffer tests
- `examples/` - standalone rendering demo

## Rendering module

The rendering subsystem consists of:

- `Color` - RGBA pixel representation
- `FrameBuffer` - CPU image storage and drawing operations
- `SdlPresenter` - SDL3 image presentation

The raycaster writes pixels into FrameBuffer.
SDL3 only displays the completed image.

See [render.md](docs/wolf/render.md) for interface conventions.
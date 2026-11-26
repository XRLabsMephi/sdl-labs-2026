# Software Graphics Labs - Raycaster

[![C++ CI](https://github.com/XRLabsMephi/sdl-labs-2026/actions/workflows/ci.yml/badge.svg?branch=Blobick_wolf_lab02)](https://github.com/XRLabsMephi/sdl-labs-2026/actions/workflows/ci.yml?query=branch%3ABlobick_wolf_lab02)

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

## Build presets

With Ninja installed, configure and build from the repository root:

```bash
cmake --preset debug
cmake --build --preset debug
```

Use the `release` preset for an optimized build or `sanitizers` with GCC or
Clang on Linux for AddressSanitizer and UBSan. Each preset uses its own
directory under `build/`. If SDL3 is installed outside the default search
paths, pass `-DCMAKE_PREFIX_PATH=/path/to/sdl3` when configuring.

## Tests

Run the full test suite after a manual build:

```bash
ctest --test-dir build --output-on-failure
```

For a preset build, run `ctest --preset debug`. To build and test in one
step after configuration, use `cmake --build --preset debug --target check`.
Use `ctest --preset debug -L render` to run the rendering tests only.

## Run the raycaster

Run from the repository root so relative paths under `assets/wolf/` resolve.
After the Debug preset build:

```bash
./build/debug/bin/graphics_lab
```

After the manual build above, use `./build/bin/graphics_lab`.

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

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for branch,
commit message and development conventions.

## Controls

- `WASD`: move (with acceleration and braking)
- `Shift + WASD` or double-tap `W`: sprint
- Hold sprint for 3 seconds: gradually build up to 2.5x speed
- `Ctrl`: drift / slide
- `Space`: dash (0.85-second cooldown)
- Right mouse button: toggle mouse-look capture (click again to release)
- Losing window focus automatically releases mouse capture
- `T`: restart at spawn and reset the run timer shown in the window title
- `Esc`: exit

# BlackHoleSim: Real-Time Gravitational Lensing

![Black Hole Simulation](./docs/demo.gif)

A real-time, Linux-native black hole visualization built with C++ and OpenGL 4.3 compute shaders.

This project simulates light bending around a black hole, renders an accretion disk with relativistic Doppler beaming, and warps a procedural background starfield through the lensing field. It was developed as an educational visualization tool and as a technical GPU programming project.

## Features

- Compute-shader-based black hole rendering
- Gravitational lensing around the event horizon
- Accretion disk with radius-based temperature falloff
- Relativistic Doppler beaming on the rotating disk
- Procedural background starfield with visible lensing distortion
- Camera showcase presets for presentation/demo use
- Linux-first CMake build with vendored GLFW

## Technical Overview

The renderer does not use traditional mesh-based scene rendering for the black hole effect.

Instead, each pixel is computed in `computeShader.glsl` by:
- generating a primary ray from the camera
- integrating the photon trajectory in a curved-space approximation
- checking whether the ray is absorbed by the black hole
- checking whether it intersects the accretion disk
- returning warped background starfield color if it escapes

The output is written into a floating-point texture and then displayed on a fullscreen quad.

## Requirements

- Linux
- OpenGL 4.3 or newer
- GPU and drivers with compute shader support
- CMake 3.16+
- GCC or Clang with C++17 support

## Build

Clone with submodules:

```bash
git clone --recurse-submodules https://github.com/AaravSubberwal/Black-Hole-Sim-OpenGL.git
cd Black-Hole-Sim-OpenGL
```

Configure and build:

```bash
cmake -S . -B build
cmake --build build
```

Run:

```bash
./build/bin/BlackHoleSimulation
```

## Controls

- `Mouse`: look around
- `W A S D`: move
- `Space`: move down
- `Left Shift`: move up
- `Esc`: quit

## Presets

- `1`: high angled orbit view
- `2`: edge-on disk view for strongest Doppler beaming
- `3`: face-on top-down view
- `4`: near-horizon close-up

## Project Structure

- `src/main.cpp`: application setup and render loop
- `src/Camera.cpp`: movement, mouse look, and camera presets
- `src/Window.cpp`: GLFW/OpenGL initialization and runtime checks
- `src/shader.cpp`: shader loading and uniform handling
- `src/BlackHole.cpp`: render target and disk parameter setup
- `res/computeShader.glsl`: black hole, accretion disk, Doppler beaming, and starfield logic
- `res/vertexShader.glsl`: fullscreen quad vertex shader
- `res/fragmentShader.glsl`: fullscreen texture display shader

## Notes

This project is Linux-first by design. Windows support is possible later, but is not the current target.

The simulation is intended as an educational and visually grounded approximation, not a full general-relativistic radiative transfer solver.

## Author

Developed by Aarav Subberwal.

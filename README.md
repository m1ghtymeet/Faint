# Moon Engine
## 1. Introduction
**Moon Engine** is a modern 3D game engine built in **C++**, designed for flexibility, performance, and extensibility.
It provides a complete workflow for creating, rendering, and scripting interactive 3D scenes with a built-in editor, a PhysX-based physics system, Lua scripting, and a deferred PBR rendering pipeline powered by **OpenGL**.

## Key Features
- Physically-Based Rendering (PBR) pipeline with deferred shading
- Shadow mapping (directional, spot, and point lights)
- PhysX integration for physics and collision simulation
- Lua scripting for game logic
- Entity Component System (ECS) architecture
- Material and scene editors
- Editor camera with full transform controls
- Full OpenGL backend
- Modular system and component design

## Supported Platforms
- Windows 10/11 (x64)

## Requirements
- OpenGL 4.5+ compatible GPU
- PhysX SDK (version used in the engine)
- Lua 5.4

## Dev Requirements
- Visual Studio 2022
- Premake5

## 2. Installation & Setup
### Step 1 – Clone the Repository
```
git clone https://github.com/YourName/Faint.git
cd Faint
```

### Step 2 – Generate Visual Studio Project
Make sure `premake5.exe` is in your PATH, then run:
```
premake5 vs2022
```
This will generate a Visual Studio solution under /build.
### Step 3 – Build the Engine
Open the solution in Visual Studio and build the Editor configuration.
### Step 4 – Run the Editor
After a successful build, run:
```
bin/Editor.exe
```
The Editor window should open with a default scene.

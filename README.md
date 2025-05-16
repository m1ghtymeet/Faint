# Faint

## Contents
- [1. What's Faint?](#1-what's-Faint)
- [2. Downloading](#2-download)
- [3. Features](#3-features)
- [4. Planed Features](#4-planned-features)
- [5. Dependencies](#5-dependencies)
- [6. Requirements](#6-system-requirements)

### 1. What's Faint?
Faint is a 3D game engine developed in C++, with support for C# scripting via Coral.
Development began in 2024 by MightyMeet with the goal of creating a flexible and powerful engine for indie and hobbyist developers.

### 2. Download
Get the latest release from the [page](https://github.com/m1ghtymeet/Faint/releases).
View all available versions [here](https://github.com/m1ghtymeet/Faint/releases).

To clone the repository using Git:
```bash
git clone https://github.com/m1ghtymeet/Faint
```

### 3. Features
- Physically-Based Rendering (PBR)
- Built-in Game Editor
- Build and Export Standalone Games
- OBJ/FBX Model Loading (via Assimp)
- Automatic detection of CPU or GPU presence

### 4. Planned Features
- Custom/Handeling Shaders
- Lua Scripting
- Custom Windows Theme

### 5. Dependencies
Faint relies on the following libraries/tools:
- Rendering: OpenGL, GLAD, ImGui
- Math: GLM
- Model/Asset Loading: Assimp tinyobj
- Scripting: Coral (C#)
- Project Generation: Premake5
- ECS: EnTT
- Physics: NVIDIA PhysX
- Audio: Soloud
- Serialization: nlohmann::json, yaml-cpp

### 6. System Requirements
- **OS:** Windows 7/10 (64-bit)
- **CPU:** x64 processor
- **RAM:** Minimum 150MB
- **GPU:** Graphics card with OpenGL 4.5 support
- **Disk Space:** 800MB

## Learning Resources
![Learn OpenGL](https://learnopengl.com/)
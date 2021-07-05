# AltCraft [![CI](https://github.com/LaG1924/AltCraft/actions/workflows/ci.yml/badge.svg?event=push)](https://github.com/LaG1924/AltCraft/actions/workflows/ci.yml)
AltCraft is implementation of [Minecraft: Java Edition (1.12.2)](https://minecraft.net)  in C++.

This is aimed at being fast, being free and being easy expandable.

# Demonstration ([more](https://github.com/LaG1924/AltCraft/discussions/69))

![screenshot_1](https://user-images.githubusercontent.com/12997935/124433999-cbd7a780-dd8c-11eb-812d-e89c65ffabc6.png)
![screenshot_2](https://user-images.githubusercontent.com/12997935/124433848-9fbc2680-dd8c-11eb-9c68-a0a9a3dcaecb.png)

Clickable video:
[![video_1_yt](https://img.youtube.com/vi/LvvsEjPvCmY/0.jpg)](https://www.youtube.com/watch?v=LvvsEjPvCmY)

# Build
1. Install CMake 3.14, C++ compiler, OpenGL.

>Windows: VS2019 and workload Desktop development with C++

>Ubuntu: `apt install cmake g++ libgl1-mesa-dev libglu1-mesa-dev`

>MacOS: `xcode-select --install` and install [CMake](https://cmake.org/download/)

2. Build AltCraft using CMake (run in directory with clonned AltCraft repo):
```
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .
```

>Windows: Just `Clone a repository` on statup screen in VS2019.

>Linux and MacOS: Alternatively, you could build and run using IDE such as CLion.

>Linux: There is a problem that Optick does not compile, so for now it needs to be disabled via the additional CMake parameter `-DOPTICK_ENABLED=OFF`.
3. Copy *.minecraft/version/{version}/{version}.jar/assets/* to *AltCraft-root/cwd/*
>You can extract .jar file as .zip archive.

>*/cwd/assets/minecraft/models/block/block.json* must be a valid path.
4. Set /cwd/ as working directory and run AC: `cd ../cwd/ && ../build/AltCraft`

# AltCraft
AltCraft is implementation of [Minecraft: Java Edition](https://minecraft.net) in C++.

This is aimed at being fast, being free and being easy expandable.
# Features and TODOs
### General
- [x] Compiles!
- [x] Some general optimizations
- [x] Some graphics and gameplay settings (mouse sensitivity, rendering distance, username, etc)
- [x] C++14
- [x] Minecraft 1.12.2
- [ ] Decouple bundled dependencies
### Original Minecraft resources support
- [x] Block models
- [ ] Block states
- [ ] Generating texture atlas at runtime
### Network
- [x] Connecting to server and receiving/transmitting of packets
- [x] Server with compression (receiving packets only)
- [ ] Mojang account auth
- [ ] Server with encryption
### Physics
- [x] Simple AABB-collision detection
- [x] Simple physics
- [ ] Advanced physics
- [ ] Optimization: Octree
### Gameplay
- [x] Player moving
- [x] Chat and inventory partially implemented
- [x] GUI based on [Dear ImGui](https://github.com/ocornut/imgui) library
- [ ] Digging
- [ ] Block placing
- [ ] Simple lighting
- [ ] Chat
- [ ] Inventory
### Graphics
- [x] OpenGL 3.3
- [x] Rendering world
- [x] Rendering entities position
- [x] Realtime blocks updating
- [ ] Rendering models of entities
- [ ] Advanced lighting and shadowing
- [ ] Optimization: Occlusion culling algorithm
- [ ] Optimization: Greedy mesh simplification

# Build
1. Install C++ compiler, CMake 3.8, SDL2, libZLIB.
>Linux: GCC7 and system package manager.

>Windows: Visual Studio 2017 and vcpkg.
3. Generate project-files for your compiler by CMake.
```
    mkdir build && cd build
    cmake ..
```
2. Copy *.minecraft/version/{version}/{version}.jar/assets/* to *AltCraft-root/cwd/*
>You can extract .jar file as .zip archive.

>*/cwd/assets/minecraft/models/block/block.json* must be a valid path.
4. Compile generated files. Content of */cwd/* will be automatically copied to directory with compiled binary.
>Linux: `make` in directory with generated files.

>Windows: Use VS2017 to open .sln file and build project.
5. Enjoy my game, AltCraft!

(PS: You may need to turn off your security app. I use Norton and it thinks some of the game files are virus.) (by Ricky Cheung)

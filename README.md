# AltCraft
AltCraft is implementation of [Minecraft: Java Edition](https://minecraft.net) in C++.

This is aimed at being fast, being free and being easy expandable.

# Build
1. Install C++ compiler, CMake 3.5, SDL2, SDL2_net, libZLIB.

>Arch: `sudo pacman -S cmake gcc sdl2 sdl2_net`

>Debian: `sudo apt install cmake g++ libsdl2-dev libsdl2-net-dev`

>Fedora: `sudo dnf install cmake gcc-c++ SDL2-devel SDL2_net-devel mesa-libGL-devel mesa-libGLU-devel zlib-devel`

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
5. Enjoy!

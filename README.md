# AltCraft [![Build Status](https://travis-ci.org/LaG1924/AltCraft.svg?branch=master)](https://travis-ci.org/LaG1924/AltCraft)
[AltCraft](https://www.youtube.com/watch?v=LvvsEjPvCmY) is implementation of [Minecraft: Java Edition](https://minecraft.net) client in C++.

This is aimed at being fast, free, portable and easy expandable.

## Features
#### Protocol
Current protocol version: [340](https://wiki.vg/index.php?title=Protocol&oldid=14204) (Minecraft 1.12.2)
- [x] Compression
    - [x] Decompression
    - [ ] Compression itself
- [ ] Encryption
- [ ] NBT tags
- [ ] Inventory
- [x] Multiple dimensions
- [x] Plugin channels

#### Client
- [x] World rendering
- [x] Texture atlas
- [x] World editing
- [x] Lightning
	- [x] Sky lightning
	- [x] Block lightning
		- [ ] Sky lightning update
- [ ] Entities models
- [ ] Sound
- [ ] Mesh optimization
- [ ] Dynamic FOV
- [ ] Changing base FOV
- [ ] Server list
- [ ] Nice GUI

#### Extensions
- [ ] Transperent protocol-level proxy
- [ ] FML protocol

## Building
1. Install C++ compiler, CMake 3.5, SDL2, SDL2_net(exclude Linux), libZLIB.

>Arch: `sudo pacman -S cmake gcc sdl2`

>Debian: `sudo apt install cmake g++ libsdl2-dev`

>Fedora: `sudo dnf install cmake gcc-c++ SDL2-devel mesa-libGL-devel mesa-libGLU-devel zlib-devel`

>Linux: GCC7 and system package manager.

>Windows: Visual Studio 2017 and vcpkg.
2. Generate project-files for your compiler by CMake.
```
    mkdir build && cd build
    cmake ..
```
3. Copy *.minecraft/version/{version}/{version}.jar/assets/* to *AltCraft-root/cwd/*
>You can extract .jar file as .zip archive.

>*/cwd/assets/minecraft/models/block/block.json* must be a valid path.
4. Compile generated files. Content of */cwd/* will be automatically copied to directory with compiled binary.
>Linux: `make` in directory with generated files.

>Windows: Use VS2017 to open .sln file and build project.
5. Enjoy!

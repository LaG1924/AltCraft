cmake_minimum_required(VERSION 3.0)
project(AltCraft)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -w -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    #set(CMAKE_CXX_FLASG "${CMAKE_CXX_FLAGS} -g -O0")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg")
    set(PLATFORM_LIBRARIES "pthread")
endif ()

set(SOURCE_FILES src/main.cpp src/packet/Field.hpp src/utility/utility.cpp src/packet/Packet.hpp src/packet/FieldParser.hpp src/network/Network.hpp src/network/Network.cpp src/network/NetworkClient.cpp
        src/network/NetworkClient.hpp src/packet/PacketBuilder.cpp src/packet/PacketBuilder.hpp src/packet/Packet.cpp src/packet/FieldParser.cpp src/packet/Field.cpp
        src/packet/PacketParser.cpp src/packet/PacketParser.hpp src/gamestate/Game.cpp
        src/gamestate/Game.hpp src/world/World.cpp src/world/World.hpp src/world/Block.cpp src/world/Block.hpp src/world/Section.cpp src/world/Section.hpp src/nbt/Nbt.hpp src/graphics/AssetManager.cpp
        src/graphics/AssetManager.hpp src/graphics/Display.cpp src/graphics/Display.hpp src/graphics/Camera3D.hpp src/graphics/Camera3D.cpp
        src/graphics/Shader.hpp src/graphics/Shader.cpp src/graphics/Texture.hpp src/graphics/Texture.cpp src/core/Core.cpp src/core/Core.hpp src/gamestate/GameState.cpp src/gamestate/GameState.hpp src/utility/Vector.hpp)

add_executable(AltCraft ${SOURCE_FILES})
target_link_libraries(AltCraft ${PLATFORM_LIBRARIES})

add_subdirectory(depedencies)
target_include_directories(AltCraft PUBLIC ./depedencies/include)
target_link_libraries(AltCraft deps)

#Setup SFML
find_package(SFML 2 COMPONENTS system window graphics network REQUIRED)
if (SFML_FOUND)
    target_link_libraries(AltCraft ${SFML_LIBRARIES})
    target_include_directories(AltCraft PUBLIC ${SFML_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "SFML not found!")
endif ()

#Setup OpenGL
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    target_link_libraries(AltCraft ${OPENGL_LIBRARIES})
    target_include_directories(AltCraft PUBLIC ${OPENGL_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "OpenGL not found!")
endif ()

#Setup Zlib
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    target_link_libraries(AltCraft ${ZLIB_LIBRARIES})
    target_include_directories(AltCraft PUBLIC ${ZLIB_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "Zlib not found!")
endif ()

cmake_minimum_required(VERSION 3.0)
project(AltCraft)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -w -Werror")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    #set(CMAKE_CXX_FLASG "${CMAKE_CXX_FLAGS} -g -O0")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg")
    set(PLATFORM_LIBRARIES "pthread")
endif ()

set(SOURCE_FILES main.cpp Field.hpp utility.cpp Packet.hpp FieldParser.hpp Network.hpp Network.cpp NetworkClient.cpp
        NetworkClient.hpp json.hpp PacketBuilder.cpp PacketBuilder.hpp Packet.cpp FieldParser.cpp Field.cpp
        PacketParser.cpp PacketParser.hpp PositionF.cpp PositionF.hpp PositionI.cpp PositionI.hpp Game.cpp
        Game.hpp World.cpp World.hpp Block.cpp Block.hpp Section.cpp Section.hpp Nbt.hpp graphics/AssetManager.cpp
        graphics/AssetManager.hpp graphics/Display.cpp graphics/Display.hpp graphics/Camera3D.hpp graphics/Camera3D.cpp
        graphics/Shader.hpp graphics/Shader.cpp graphics/Texture.hpp graphics/Texture.cpp)

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


#[[
#Setup SFML
find_package(SFML 2 COMPONENTS system network graphics window REQUIRED)
include_directories(${SFML_INCLUDE_DIR})
if (NOT SFML_FOUND)
    message(FATAL_ERROR "SFML not found!")
endif ()

#Setup Zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})
if (NOT ZLIB_FOUND)
    message(FATAL_ERROR "Zlib not found!")
endif ()

#Setup GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
if (NOT GLEW_FOUND)
    message(FATAL_ERROR "GLEW not found!")
endif ()

#Setup OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
if (NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL not found!")
endif ()

set(SOURCE_FILES main.cpp Field.hpp utility.cpp Packet.hpp FieldParser.hpp Network.hpp Network.cpp NetworkClient.cpp
        NetworkClient.hpp json.hpp PacketBuilder.cpp PacketBuilder.hpp Packet.cpp FieldParser.cpp Field.cpp
        PacketParser.cpp PacketParser.hpp PositionF.cpp PositionF.hpp PositionI.cpp PositionI.hpp Game.cpp
        Game.hpp World.cpp World.hpp Block.cpp Block.hpp Section.cpp Section.hpp Nbt.hpp graphics/AssetManager.cpp
        graphics/AssetManager.hpp)

add_executable(AltCraft ${SOURCE_FILES})
target_link_libraries(AltCraft ${PLATFORM_LIBRARIES} ${SFML_LIBRARIES} ${ZLIB_LIBRARIES} ${SOIL_LIBRARY}
        ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
]]
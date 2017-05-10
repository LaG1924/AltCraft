cmake_minimum_required(VERSION 3.0)
project(AltCraft)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors -w -Werror")
    set(CMAKE_CXX_FLASG "${CMAKE_CXX_FLAGS} -g -O0")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg")
    set(PLATFORM_LIBRARIES "pthread")
endif ()

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

#Setup SOIL
find_package(SOIL REQUIRED)
include_directories(${SOIL_INCLUDE_DIRS})
if (NOT SOIL_FOUND)
    message(FATAL_ERROR "SOIL not found!")
endif ()

#Setup GLFW3
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
if (NOT GLFW_FOUND)
    message(FATAL_ERROR "GLFW3 not found!")
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
        graphics/AssetManager.hpp graphics/Display.cpp graphics/Display.hpp graphics/Shader.cpp graphics/Shader.hpp)

add_executable(AltCraft ${SOURCE_FILES})
target_link_libraries(AltCraft ${PLATFORM_LIBRARIES} ${SFML_LIBRARIES} ${ZLIB_LIBRARIES} ${SOIL_LIBRARY}
        ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

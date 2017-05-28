cmake_minimum_required(VERSION 3.0)
project(AltCraft)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

##########
# SOURCES
##########

set(SRC_CORE src/core/Core.hpp src/core/Core.cpp src/core/AssetManager.hpp src/core/AssetManager.cpp)
set(SRC_GAMESTATE src/gamestate/GameState.hpp src/gamestate/GameState.cpp)
set(SRC_GRAPHICS src/graphics/Texture.cpp src/graphics/Shader.cpp src/graphics/Camera3D.cpp)
set(SRC_GUI src/gui/Gui.cpp src/gui/Widget.cpp)
set(SRC_NBT src/nbt/Nbt.hpp)
set(SRC_NETWORK src/network/Network.cpp src/network/NetworkClient.cpp)
set(SRC_PACKET src/packet/Field.cpp src/packet/FieldParser.cpp src/packet/Packet.cpp src/packet/PacketBuilder.cpp
        src/packet/PacketParser.cpp)
set(SRC_UTILITY src/utility/Vector.hpp src/utility/utility.cpp)
set(SRC_WORLD src/world/Block.cpp src/world/Section.cpp src/world/World.cpp)

set(SOURCE_FILES src/main.cpp ${SRC_CORE} ${SRC_GAMESTATE} ${SRC_GRAPHICS} ${SRC_GUI} ${SRC_GRAPHICS} ${SRC_GUI}
        ${SRC_NBT} ${SRC_NETWORK} ${SRC_PACKET} ${SRC_UTILITY} ${SRC_WORLD})

add_executable(AltCraft ${SOURCE_FILES})

################
# CONFIGURATION
################
set(LOGGER_DEFINITIONS "-DELPP_THREAD_SAFE -DELPP_FEATURE_CRASH_LOG")
add_definitions(${LOGGER_DEFINITIONS})
#-DELPP_HANDLE_SIGABRT

#Set compiler's flags and setup platfrom-dependent libraries
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -w -Werror -Wall -Wextra")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    #set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(PLATFORM_LIBRARIES "pthread" "stdc++fs")
endif ()


############
# LIBRARIES
############

target_link_libraries(AltCraft ${PLATFORM_LIBRARIES})

#Setup dependencies (GLEW, glm, nlohmann/json, easylogging++)
add_subdirectory(depedencies)
target_include_directories(AltCraft PUBLIC ./depedencies/include)
target_link_libraries(AltCraft deps)

#Setup SFML
find_package(SFML 2.3 COMPONENTS system window graphics network REQUIRED)
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


include(cotire)
set_target_properties(AltCraft PROPERTIES
        COTIRE_PREFIX_HEADER_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/depedencies")
cotire(AltCraft)
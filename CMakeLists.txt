cmake_minimum_required(VERSION 3.0)
project(AltCraft)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

##########
# SOURCES
##########

#[[set(SRC_CORE src/core/Core.cpp src/core/AssetManager.cpp)
set(SRC_GRAPHICS src/graphics/Texture.cpp src/graphics/Shader.cpp src/graphics/RenderSection.cpp src/graphics/Gui.cpp
        src/graphics/Widget.cpp)
set(SRC_NBT include/Nbt.hpp)
set(SRC_NETWORK src/network/Network.cpp src/network/NetworkClient.cpp src/network/Socket.cpp src/network/Stream.cpp)
set(SRC_WORLD src/world/Block.cpp src/world/Section.cpp src/world/World.cpp src/world/Collision.cpp
        src/world/GameState.cpp)*#

set(SOURCE_FILES src/main.cpp ${SRC_CORE} ${SRC_GRAPHICS} ${SRC_NETWORK} ${SRC_WORLD})]]#
file(GLOB_RECURSE SOURCES "./src/*.cpp")
file(GLOB_RECURSE HEADERS "./include/*.hpp")

add_executable(AltCraft ${HEADERS} ${SOURCES})

target_include_directories(AltCraft PUBLIC include/)

################
# CONFIGURATION
################

#Configure easlylogging++
set(LOGGER_DEFINITIONS "-DELPP_THREAD_SAFE")
if (LINUX)
    set(LOGGER_DENITIONS "${LOGGER_DEFINITIPNS} -DELPP_FEATURE_CRASH_LOG -DELPP_STL_LOGGING")
endif ()
add_definitions(${LOGGER_DEFINITIONS})

#Set compiler's flags and setup platfrom-dependent libraries
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Werror")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    #set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -p")
    set(PLATFORM_LIBRARIES "pthread" "stdc++fs")
endif ()
target_link_libraries(AltCraft ${PLATFORM_LIBRARIES})

############
# LIBRARIES
############

#Setup dependencies (GLEW, glm, nlohmann/json, easylogging++)
add_subdirectory(depedencies)
target_include_directories(AltCraft PUBLIC ./depedencies/include)
target_link_libraries(AltCraft deps)

#Setup SFML
set(SFML_ROOT "${CMAKE_SOURCE_DIR}/SFML-2.4.2")
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
#find_package(ZLIB REQUIRED)
#if (ZLIB_FOUND)
#    target_link_libraries(AltCraft ${ZLIB_LIBRARIES})
#    target_include_directories(AltCraft PUBLIC ${ZLIB_INCLUDE_DIR})
#else ()
#    message(FATAL_ERROR "Zlib not found!")
#endif ()

#################
# COPY RESOURCES
#################

add_custom_command(TARGET AltCraft PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/cwd $<TARGET_FILE_DIR:AltCraft>)
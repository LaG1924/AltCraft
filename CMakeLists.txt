cmake_minimum_required(VERSION 3.5)
project(AltCraft)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors -w -Werror")
set(CMAKE_CXX_FLASG "${CMAKE_CXX_FLAGS} -g -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

#Setup SFML
find_package(SFML 2 COMPONENTS system network graphics window REQUIRED)
include_directories(${SFML_INCLUDE_DIR})
if (NOT SFML_FOUND)
    message(ERROR "SFML not found!")
endif ()

#Setup Zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})
if (NOT ZLIB_FOUND)
    message(ERROR "Zlib not found!")
endif ()


set(SOURCE_FILES main.cpp Field.hpp utility.cpp Packet.hpp FieldParser.hpp Network.hpp Network.cpp NetworkClient.cpp
        NetworkClient.hpp json.hpp PacketBuilder.cpp PacketBuilder.hpp Packet.cpp FieldParser.cpp Field.cpp
        PacketParser.cpp PacketParser.hpp PositionF.cpp PositionF.hpp PositionI.cpp PositionI.hpp Game.cpp
        Game.hpp World.cpp World.hpp Block.cpp Block.hpp Display.cpp Display.hpp Section.cpp Section.hpp Nbt.hpp)

add_executable(AltCraft ${SOURCE_FILES})
target_link_libraries(AltCraft ${SFML_LIBRARIES} ${ZLIB_LIBRARIES} pthread)
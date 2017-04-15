cmake_minimum_required(VERSION 3.5)
project(AltCraft)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS "-fsanitize=address")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

#Setup SFML
find_package(SFML 2 COMPONENTS system network graphics window REQUIRED)
include_directories(${SFML_INCLUDE_DIR})
if (NOT SFML_FOUND)
    message(ERROR "SFML not found!")
endif ()

set(SOURCE_FILES main.cpp Field.hpp utility.cpp Packet.hpp FieldParser.hpp Network.hpp Network.cpp NetworkClient.cpp
        NetworkClient.hpp json.h PacketBuilder.cpp PacketBuilder.hpp Packet.cpp FieldParser.cpp Field.cpp
        PacketParser.cpp PacketParser.hpp PositionF.cpp PositionF.hpp PositionI.cpp PositionI.hpp Game.cpp
        Game.hpp World.cpp World.hpp Block.cpp Block.hpp Display.cpp Display.hpp)
#file(GLOB SOURCE_FILES "*.h" "*.hpp" "*.cpp")
add_executable(AltCraft ${SOURCE_FILES})
target_link_libraries(AltCraft ${SFML_LIBRARIES})
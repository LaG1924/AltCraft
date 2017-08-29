cmake_minimum_required(VERSION 3.8)
project(AltCraft)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(externalProject)

##########
# SOURCES
##########

file(GLOB_RECURSE SOURCES "./src/*.cpp")
file(GLOB_RECURSE HEADERS "./src/*.hpp")

add_executable(AltCraft ${HEADERS} ${SOURCES})

################
# CONFIGURATION
################

#Configure easlylogging++
set(LOGGER_DEFINITIONS "-DELPP_THREAD_SAFE -DELPP_FEATURE_PERFORMANCE_TRACKING")
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

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

target_link_libraries(AltCraft ${PLATFORM_LIBRARIES})

############
# LIBRARIES
############

#Setup dependencies (GLEW, glm, nlohmann/json, easylogging++)
add_subdirectory(depedencies)
target_include_directories(AltCraft PUBLIC ./depedencies/include)
target_link_libraries(AltCraft deps)

#Setup SDL2
find_package(SDL2 REQUIRED)
target_link_libraries(AltCraft ${SDL2_LIBRARY})
target_include_directories(AltCraft PUBLIC ${SDL2_INCLUDE_DIR})

#Setup SDL2_net
find_package(SDL2_net REQUIRED)
target_link_libraries(AltCraft ${SDL2_NET_LIBRARIES})
target_include_directories(AltCraft PUBLIC ${SDL2_NET_INCLUDE_DIRS})

#Setup SDL2_image
find_package(SDL2_image REQUIRED)
target_link_libraries(AltCraft ${SDL2_IMAGE_LIBRARIES})
target_include_directories(AltCraft PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})

#Setup OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(AltCraft ${OPENGL_LIBRARIES})
target_include_directories(AltCraft PUBLIC ${OPENGL_INCLUDE_DIRS})


#################
# COPY RESOURCES
#################

add_custom_command(TARGET AltCraft PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/cwd $<TARGET_FILE_DIR:AltCraft>)
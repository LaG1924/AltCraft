cmake_minimum_required(VERSION 3.5)
project(AltCraft)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
endif()

##########
# SOURCES
##########

file(GLOB_RECURSE MANIFEST "./src/*.manifest")
file(GLOB_RECURSE SOURCES "./src/*.cpp")
file(GLOB_RECURSE HEADERS "./src/*.hpp")

add_executable(AltCraft ${HEADERS} ${SOURCES} ${MANIFEST})

################
# CONFIGURATION
################

option(ENABLE_OPTICK "Enable profiler \"Optick \"?" OFF)

#Configure Optick
if (${ENABLE_OPTICK})
	add_definitions(-DUSE_OPTICK=1)
else()
	add_definitions(-DUSE_OPTICK=0)
endif()

#Configure easlylogging++
set(LOGGER_DEFINITIONS "-DELPP_THREAD_SAFE -DELPP_FEATURE_PERFORMANCE_TRACKING")
if (LINUX)
    set(LOGGER_DENITIONS "${LOGGER_DEFINITIPNS} -DELPP_FEATURE_CRASH_LOG -DELPP_STL_LOGGING")
endif ()
add_definitions(${LOGGER_DEFINITIONS})

#Set compiler's flags and setup platfrom-dependent libraries
if (CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wno-error=unused-parameter")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -p")
    set(PLATFORM_LIBRARIES "stdc++fs")
endif ()

if(MSVC AND CMAKE_BUILD_TYPE MATCHES Release)
	target_compile_definitions(AltCraft PRIVATE USE_SDL_MAIN)
	set_target_properties(AltCraft PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif(MSVC AND CMAKE_BUILD_TYPE MATCHES Release)

target_link_libraries(AltCraft ${PLATFORM_LIBRARIES})

set(CPM_USE_LOCAL_PACKAGES ON)
include(get_cpm)

############
# LIBRARIES
############

#Setup dependencies (GLEW, glm, nlohmann/json, easylogging++)
add_subdirectory(external)
target_include_directories(AltCraft PUBLIC external/include)
target_link_libraries(AltCraft deps)

target_include_directories(AltCraft PUBLIC external/optick)
if (${ENABLE_OPTICK})
	target_link_libraries(AltCraft optick)
endif()

#Setup SDL2
find_package(SDL2 REQUIRED)
target_link_libraries(AltCraft ${SDL2_LIBRARY})
target_include_directories(AltCraft PUBLIC ${SDL2_INCLUDE_DIR})

#Setup SDL2_net
find_package(SDL2_net REQUIRED)
target_link_libraries(AltCraft ${SDL2_NET_LIBRARIES})
target_include_directories(AltCraft PUBLIC ${SDL2_NET_INCLUDE_DIRS})

#Setup OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(AltCraft ${OPENGL_LIBRARIES})
target_include_directories(AltCraft PUBLIC ${OPENGL_INCLUDE_DIRS})

#Setup Zlib
find_package(ZLIB REQUIRED)
target_link_libraries(AltCraft ${ZLIB_LIBRARIES})
target_include_directories(AltCraft PUBLIC ${ZLIB_INCLUDE_DIRS})

#Setup Lua
CPMAddPackage(
  NAME lua
  GIT_REPOSITORY https://github.com/lua/lua
  VERSION 5.4.3
  DOWNLOAD_ONLY YES
)
if(lua_ADDED)
  # lua has no CMakeLists, so we create our own target
  file(GLOB lua_sources ${lua_SOURCE_DIR}/*.c)
  list(REMOVE_ITEM lua_sources "${lua_SOURCE_DIR}/lua.c" "${lua_SOURCE_DIR}/luac.c" "${lua_SOURCE_DIR}/onelua.c")
  add_library(lua STATIC ${lua_sources})
  target_include_directories(lua SYSTEM PUBLIC $<BUILD_INTERFACE:${lua_SOURCE_DIR}>)
endif()

#Setup Sol2
CPMAddPackage(
  NAME sol2
  GIT_REPOSITORY https://github.com/ThePhD/sol2
  VERSION 3.2.2
  DOWNLOAD_ONLY YES
)
if(sol2_ADDED)
  add_library(sol2 INTERFACE IMPORTED)
  target_include_directories(sol2 SYSTEM INTERFACE ${sol2_SOURCE_DIR}/include)
  target_link_libraries(sol2 INTERFACE lua)
endif()
target_link_libraries(AltCraft sol2)

#################
# COPY RESOURCES
#################

add_custom_command(TARGET AltCraft PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/cwd $<TARGET_FILE_DIR:AltCraft>)

set_target_properties(
        AltCraft PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cwd")
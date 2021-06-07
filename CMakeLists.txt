cmake_minimum_required(VERSION 3.9)
project(AltCraft)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(get_cpm)

if (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
endif()

############
# LIBRARIES
############


find_package(Threads)


find_package(OpenGL REQUIRED)


CPMAddPackage("gh:g-truc/glm@0.9.9.8#0.9.9.8")


CPMAddPackage("gh:bombomby/optick@1.3.1.0#1.3.1.0")


CPMAddPackage(
    GITHUB_REPOSITORY Perlmint/glew-cmake
    VERSION 2.2.0
    GIT_TAG glew-cmake-2.2.0
    OPTIONS
    "glew-cmake_BUILD_SHARED OFF"
)


CPMAddPackage(
    NAME json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.9.1
    OPTIONS
    "JSON_BuildTests OFF"
)


CPMAddPackage(
    NAME SDL2
    GITHUB_REPOSITORY libsdl-org/SDL
    VERSION 2.0.14
    GIT_TAG release-2.0.14
    OPTIONS
    "SDL_SHARED ON"
    "SDL_STATIC OFF"
)


CPMAddPackage(
    NAME easyloggingpp
    GITHUB_REPOSITORY amrayn/easyloggingpp
    VERSION 9.97.0
    OPTIONS "build_static_lib ON"
)
target_include_directories(easyloggingpp PUBLIC ${easyloggingpp_SOURCE_DIR}/src)
target_compile_definitions(easyloggingpp PUBLIC ELPP_THREAD_SAFE ELPP_FEATURE_PERFORMANCE_TRACKING)
if (LINUX)
    target_compile_definitions(easyloggingpp PUBLIC ELPP_FEATURE_CRASH_LOG ELPP_STL_LOGGING)
endif ()


CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG ea2f937a01ce39795ab02b6c6e30173b4f1ed46c
    DOWNLOAD_ONLY TRUE
)
if(stb_ADDED)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endif()


CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    VERSION 1.52
    DOWNLOAD_ONLY TRUE
)
if(imgui_ADDED)
    file(GLOB imgui_sources ${imgui_SOURCE_DIR}/*.cpp)
    add_library(imgui STATIC ${imgui_sources})
    target_include_directories(imgui SYSTEM PUBLIC $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>)
endif()


CPMAddPackage(
    NAME SDL2_net
    GITHUB_REPOSITORY libsdl-org/SDL_net
    VERSION 2.0.1
    GIT_TAG release-2.0.1
    DOWNLOAD_ONLY YES
)
if(SDL2_net_ADDED)
    file(GLOB SDL2_net_sources ${SDL2_net_SOURCE_DIR}/*.c)
    list(REMOVE_ITEM SDL2_net_sources "${SDL2_net_SOURCE_DIR}/chatd.c")
    add_library(SDL2_net STATIC ${SDL2_net_sources})
    target_link_libraries(SDL2_net SDL2)
    IF (WIN32)
        target_link_libraries(SDL2_net ws2_32 iphlpapi)
    ENDIF()
    target_include_directories(SDL2_net SYSTEM PUBLIC $<BUILD_INTERFACE:${SDL2_net_SOURCE_DIR}>)
endif()


CPMAddPackage(
    NAME zlib
    GITHUB_REPOSITORY madler/zlib
    VERSION 1.2.11
    DOWNLOAD_ONLY ON
)
if(zlib_ADDED)
    file(GLOB zlib_sources ${zlib_SOURCE_DIR}/*.c)
    add_library(zlib STATIC ${zlib_sources})

    include(CheckIncludeFile)
    check_include_file(unistd.h Z_HAVE_UNISTD_H)
    if(Z_HAVE_UNISTD_H)
        target_compile_definitions(zlib PRIVATE Z_HAVE_UNISTD_H)
    endif()

    if(MSVC)
        target_compile_definitions(zlib PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
    endif()
    target_include_directories(zlib PUBLIC ${zlib_SOURCE_DIR})
endif()


CPMAddPackage(
    NAME lua
    GITHUB_REPOSITORY lua/lua
    VERSION 5.4.3
    DOWNLOAD_ONLY YES
)
if(lua_ADDED)
    file(GLOB lua_sources ${lua_SOURCE_DIR}/*.c)
    list(REMOVE_ITEM lua_sources "${lua_SOURCE_DIR}/lua.c" "${lua_SOURCE_DIR}/luac.c" "${lua_SOURCE_DIR}/onelua.c")
    add_library(lua STATIC ${lua_sources})
    target_include_directories(lua SYSTEM PUBLIC $<BUILD_INTERFACE:${lua_SOURCE_DIR}>)
endif()


CPMAddPackage(
    NAME sol2
    GITHUB_REPOSITORY ThePhD/sol2
    VERSION 3.2.2
    DOWNLOAD_ONLY YES
)
if(sol2_ADDED)
    add_library(sol2 INTERFACE IMPORTED)
    target_include_directories(sol2 SYSTEM INTERFACE ${sol2_SOURCE_DIR}/include)
    target_link_libraries(sol2 INTERFACE lua)
endif()


###########
# AltCraft
###########

file(GLOB ac_manifest "./src/*.manifest")
file(GLOB ac_sources "./src/*.cpp")

add_executable(AltCraft ${ac_sources} ${ac_manifest})

target_compile_features(AltCraft PRIVATE cxx_std_17)

target_link_libraries(AltCraft
    Threads::Threads
    OpenGL::GL
    glm
    stb
    easyloggingpp
    libglew_static
    nlohmann_json::nlohmann_json
    imgui
    SDL2
    SDL2main
    SDL2_net
    OptickCore
    zlib
    sol2
)

set_target_properties(AltCraft SDL2 OptickCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(AltCraft PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/cwd/
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AltCraft)

if(MSVC AND CMAKE_BUILD_TYPE MATCHES Release)
    set_target_properties(AltCraft PROPERTIES WIN32_EXECUTABLE ON)
endif()

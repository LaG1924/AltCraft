cmake_minimum_required(VERSION 3.5)

project(deps)

file(GLOB SOURCES "./src/*")
file(GLOB_RECURSE HEADERS "./include/*")

include_directories(./include/)

add_library(deps STATIC ${SOURCES} ${HEADERS})

file(GLOB OPTICK_SRC "./optick/*.cpp")
add_library(optick STATIC ${OPTICK_SRC})
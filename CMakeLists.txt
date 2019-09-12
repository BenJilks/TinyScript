cmake_minimum_required(VERSION 2.6)
project(TinyScript)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O")
set(CMAKE_C_FLAGS "-O3")
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/compiler/include)
include_directories(${PROJECT_SOURCE_DIR}/vm/include)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/compiler/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/compiler/src/Parser/*.cpp"
        "${PROJECT_SOURCE_DIR}/compiler/src/CodeGen/TinyVM/*.cpp"
        "${PROJECT_SOURCE_DIR}/compiler/src/CodeGen/C/*.cpp"
        "${PROJECT_SOURCE_DIR}/vm/src/*.c"
        "${PROJECT_SOURCE_DIR}/TinyScript.cpp"
)

add_executable(TinyScript ${all_SRCS})

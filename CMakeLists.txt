cmake_minimum_required(VERSION 2.8)

file(GLOB dino2-unpacker_SRC
    "*.h"
    "*.cpp"
    "converter/*.h"
    "converter/*.cpp"
    "unpacker/*.h"
    "unpacker/*.cpp"
)

project(dino2-unpacker)
add_executable(${PROJECT_NAME} ${dino2-unpacker_SRC})

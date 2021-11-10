cmake_minimum_required(VERSION 3.0)
project(dino2-unpacker)

file(GLOB dino2-unpacker_SRC
    "*.h"
    "*.cpp"
    "converter/*.h"
    "converter/*.cpp"
    "unpacker/*.h"
    "unpacker/*.cpp"
)

set (CMAKE_CXX_FLAGS "-std=c++17 ")
find_package(Qt5 REQUIRED Gui Core)

add_executable(${PROJECT_NAME} ${dino2-unpacker_SRC})
target_link_libraries(${PROJECT_NAME} Qt5::Gui)

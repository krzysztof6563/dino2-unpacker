cmake_minimum_required(VERSION 3.5)
project(dino2-unpacker)

file(GLOB dino2-unpacker_SRC
    "*.h"
    "*.cpp"
    "converter/*.h"
    "converter/*.cpp"
    "unpacker/*.h"
    "unpacker/*.cpp"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 REQUIRED Gui Core)

add_executable(${PROJECT_NAME} ${dino2-unpacker_SRC})
target_link_libraries(${PROJECT_NAME} Qt6::Gui)

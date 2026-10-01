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

include(CTest)
if(BUILD_TESTING)
    add_executable(animation_decoder_test tests/animation_decoder.cpp unpacker/DC2Animations.cpp)
    set(E30_TEST_BLOCK "" CACHE FILEPATH "User-supplied decompressed E30 model block")
    if(E30_TEST_BLOCK)
        add_test(NAME e30_animation_decoder COMMAND animation_decoder_test "${E30_TEST_BLOCK}")
    endif()
endif()

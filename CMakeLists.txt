cmake_minimum_required(VERSION 3.26)
project(bagel26)

set(CMAKE_CXX_STANDARD 20)

add_executable(bagel26
        main.cpp
        Utils.h
        Utils.cpp
        Game.h 
        Game.cpp
 "Utils.h" "Utils.cpp")
target_include_directories(bagel26 PUBLIC
    SDL/include
    SDL_image/include
)
set(SDL_STATIC ON)
set(SDL_SHARED OFF)
add_subdirectory(SDL)
target_link_libraries(bagel26 PUBLIC SDL3-static)

set(BUILD_SHARED_LIBS OFF)
set(SDLIMAGE_VENDORED OFF)
add_subdirectory(SDL_image)
target_link_libraries(bagel26 PUBLIC SDL3_image-static)
cmake_minimum_required(VERSION 3.10)
project(2DShooter2)

find_path(RL_INCLUDE_DIR NAMES raylib.h PATHS ./raylib/include)
include_directories(${RL_INCLUDE_DIR})
find_library(RL_LIBRARY NAMES raylib PATHS ./raylib/lib)

set(SOURCE_FILES
    main.c
    gif_player.c
    gif_player.h
    frame_animator.h
    frame_animator.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${RL_LIBRARY})
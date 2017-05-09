cmake_minimum_required(VERSION 3.3)

project (pong_learn)

add_subdirectory("contrib/SDL-mirror")

include_directories("include")
add_subdirectory("src")

cmake_minimum_required(VERSION 3.3)

project (pong_learn)

add_subdirectory("contrib/SDL-mirror")

set(CMAKE_CXX_STANDARD 11)
include_directories("include")
include_directories("contrib/nn_cpp/include")
add_subdirectory("src")

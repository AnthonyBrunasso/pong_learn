file(GLOB sources "*.cpp" "../include/*.h")

add_executable(pong_learn ${sources})
target_link_libraries(pong_learn)

file(GLOB sources "*.cpp" "../include/*.h")

include_directories("../contrib/SDL-mirror/include")
add_executable(pong_learn ${sources})
target_link_libraries(pong_learn SDL2-static)

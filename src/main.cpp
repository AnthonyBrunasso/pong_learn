#include <iostream>
#include <chrono>

#define SDL_MAIN_HANDLED
#include <SDL.h>

int main(int argc, char* argv[]) {


    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Learn Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 720, 480, SDL_WINDOW_SHOWN);

    if (!window) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    double t = 0.0;
    auto current = std::chrono::system_clock::now();

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                return 0;
            }
        }

        auto new_time = std::chrono::system_clock::now();
        auto frame_time = new_time - current;
        current = new_time;

        SDL_RenderClear(renderer);

        std::cout << "frame_time: " << frame_time.count() << std::endl;
        t += frame_time.count();

        SDL_RenderPresent(renderer);
    }

    return 0;
}

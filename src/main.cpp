#include <iostream>
#include <chrono>

#define SDL_MAIN_HANDLED
#include <SDL.h>

static const int WINDOW_WIDTH = 720;
static const int WINDOW_HEIGHT = 480;
static const float PADDLE_SPEED = 4.0f;


void update_simulation(SDL_Rect& p1, SDL_Rect& p2, double dt) {
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_W] && p1.y > 0) {
        p1.y -= dt * PADDLE_SPEED;
    }

    if (key_state[SDL_SCANCODE_S] && p1.y < WINDOW_HEIGHT - p1.h) {
        p1.y += dt * PADDLE_SPEED;
    }

    if (key_state[SDL_SCANCODE_UP] && p2.y > 0) {
        p2.y -= dt * PADDLE_SPEED;
    }

    if (key_state[SDL_SCANCODE_DOWN] && p2.y < WINDOW_HEIGHT - p2.h) {
        p2.y += dt * PADDLE_SPEED;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Learn Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    double t = 0.0;
    auto current = std::chrono::system_clock::now().time_since_epoch();

    SDL_Rect p1; p1.x = 0; p1.y = WINDOW_HEIGHT / 2 - 70; p1.w = 30; p1.h = 70;
    SDL_Rect p2; p2.x = WINDOW_WIDTH - 30; p2.y = WINDOW_HEIGHT / 2 - 70; p2.w = 30; p2.h = 70;

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT: {
                return 0;
            }
            default: {
                break;
            }
            }
        }

        auto new_time = std::chrono::system_clock::now().time_since_epoch();
        double frame_time = (new_time.count() - current.count()) / 100000.0;
        current = new_time;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        update_simulation(p1, p2, frame_time);
        t += frame_time;

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &p1);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &p2);

        SDL_RenderPresent(renderer);
    }

    return 0;
}

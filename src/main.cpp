#include <iostream>
#include <chrono>
#include <cmath>

#define SDL_MAIN_HANDLED
#include <SDL.h>

static const int WINDOW_WIDTH = 720;
static const int WINDOW_HEIGHT = 480;
static const float PADDLE_SPEED = 4.0f;
static const int PADDLE_WIDTH = 30;
static const int PADDLE_HEIGHT = 70;
// Dimension of bouncing square.
static const int PONG_WIDTH = 15;
static const int PONG_HEIGHT = 15;
static const float PONG_SPEED = 2.0f;

void update_simulation(SDL_Rect& p1, SDL_Rect& p2, SDL_Rect& pong, float& xv, float& yv, double dt) {
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

    static int x_valid = pong.x;
    static int y_valid = pong.y;

    if (SDL_HasIntersection(&p1, &pong) || SDL_HasIntersection(&p2, &pong)) {
        xv *= -1;
        yv *= -1;

        pong.x = x_valid;
        pong.y = y_valid;
    }
    else {
        x_valid = pong.x;
        y_valid = pong.y;
    }

    //std::cout << dt * xv << std::endl;

    pong.x += dt * xv;
    pong.y += dt * yv;
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
    double dt = 1.0;
    double acc = 0.0;

    typedef std::chrono::duration<double> dsec;

    auto current = std::chrono::system_clock::now();

    SDL_Rect p1; p1.x = 0; p1.y = WINDOW_HEIGHT / 2 - PADDLE_HEIGHT; p1.w = PADDLE_WIDTH; p1.h = PADDLE_HEIGHT;
    SDL_Rect p2; p2.x = WINDOW_WIDTH - PADDLE_WIDTH; p2.y = WINDOW_HEIGHT / 2 - PADDLE_HEIGHT; p2.w = PADDLE_WIDTH; p2.h = PADDLE_HEIGHT;
    SDL_Rect pong; pong.x = WINDOW_WIDTH / 2; pong.y = WINDOW_HEIGHT / 2 - PONG_HEIGHT; pong.w = PONG_WIDTH; pong.h = PONG_HEIGHT;
    float xv = PONG_SPEED, yv = 0.0;

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

        auto new_time = std::chrono::system_clock::now();
        dsec frame_time = new_time - current;
        current = new_time;

        acc += frame_time.count() * 100;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        while (acc >= dt) {
            update_simulation(p1, p2, pong, xv, yv, dt);
            acc -= dt;
            t += dt;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &p1);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &p2);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &pong);

        SDL_RenderPresent(renderer);
    }

    return 0;
}

#include <iostream>
#include <chrono>
#include <random>
#include <cmath>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <vector>

#include "neural_net.h"

static const int WINDOW_WIDTH = 640;
static const int WINDOW_HEIGHT = 480;
static const float PADDLE_SPEED = 3.0f;
static const int PADDLE_WIDTH = 10;
static const int PADDLE_HEIGHT = 80;
// Dimension of bouncing square.
static const int PONG_WIDTH = 15;
static const int PONG_HEIGHT = 15;
static const float PONG_SPEED = 5.0f;
static const int GRID_Y_COUNT = 15;
static const int GRID_X_COUNT = 15;
static const int WIDTH_SIZE = WINDOW_WIDTH / GRID_X_COUNT;
static const int HEIGHT_SIZE = WINDOW_HEIGHT / GRID_Y_COUNT;
static const float LEARNING_RATE = 0.0001f;
// -1 for left side, 1 for right side
static int _winner = 0;
static int _gamesplayed = 0;
static int _nn_winner = 0;
static int _ai_winner = 0;

static std::vector<Matrix<float> > _w0_updates;
static std::vector<std::vector<float> > _b0_updates;

static std::vector<Matrix<float> > _w1_updates;
static std::vector<std::vector<float> > _b1_updates;

//static Matrix<float> _weight_update(1, GRID_X_COUNT * GRID_Y_COUNT);
//static std::vector<float> _bias_update(1);
static std::vector<float> _grid(GRID_X_COUNT * GRID_Y_COUNT);
static const float NORM = sqrt(GRID_X_COUNT * GRID_Y_COUNT);

void set_entry(const SDL_Rect& rect, bool is_paddle=true) {
    int x = rect.x / WIDTH_SIZE;
    int y = rect.y / HEIGHT_SIZE;
    _grid[x + y * GRID_X_COUNT] = 1.0f;

    if (is_paddle) {
        _grid[x + (y + 1) * GRID_X_COUNT] = 1.0f;
        //_grid[x + (y + 2) * GRID_X_COUNT] = 1.0f;
    }
}

void update_grid(SDL_Rect& p1, SDL_Rect& p2, const SDL_Rect& pong) {
    // Turn all 1.0s into 0.5s and 0.5s into 0.0s to encode movement.
    for (auto& v : _grid) {
        if (v == 0.8f) {
           v = 0.0f;
        }
        if (v == 1.0f) {
            v = 0.8f;
        }
    }

    set_entry(p1);
    set_entry(p2);
    set_entry(pong, false);
}

void update_simulation(SDL_Rect& p1, SDL_Rect& p2, SDL_Rect& pong, float& xv, float& yv, double dt, NeuralNet<float>& nn) {
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    float dy_p1 = 0.0f;
    float dy_p2 = 0.0f;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> ds(0.0f, 1.0f);

    update_grid(p1, p2, pong);

    std::vector<float> result = nn.predict(_grid);

    float value = result.front();

   // std::cout << 1.0f - value << " " << value << std::endl;

    std::discrete_distribution<> d({ 1.0f - value, value });
    int chosen = d(gen);

    std::vector<Matrix<float> > weight;
    Vector2<float> bias;
    nn.backprop(_grid, { (float)chosen }, weight, bias);

    _w0_updates.push_back(weight.front());
    _b0_updates.push_back(bias.front());

    _w1_updates.push_back(weight.back());
    _b1_updates.push_back(bias.back());

    //_weight_update += weight.front();
    //_bias_update = _component_sum(_bias_update, bias.front());

    if (chosen == 1 && p1.y > 0) {
        p1.y -= dt * PADDLE_SPEED;
        dy_p1 -= dt * PADDLE_SPEED * ds(gen);
    }

    if (chosen == 0  && p1.y < WINDOW_HEIGHT - p1.h) {
        p1.y += dt * PADDLE_SPEED;
        dy_p1 += dt * PADDLE_SPEED * ds(gen);
    }

    if (p2.y > pong.y + p2.h / 2 && p2.y > 0) {
        p2.y -= dt * PADDLE_SPEED;
        dy_p2 -= dt * PADDLE_SPEED * ds(gen);
    }

    if (p2.y < pong.y - p2.h / 2 && p2.y < WINDOW_HEIGHT - p2.h) {
        p2.y += dt * PADDLE_SPEED;
        dy_p2 += dt * PADDLE_SPEED * ds(gen);
    }

    static int x_valid = pong.x;
    static int y_valid = pong.y;

    p1.w = 1; p2.w = 1;

    bool hit_p1 = SDL_HasIntersection(&p1, &pong);
    bool hit_p2 = SDL_HasIntersection(&p2, &pong);

    p1.w = PADDLE_WIDTH; p2.w = PADDLE_WIDTH;

    if (hit_p1) {
        // Reward simply hitting the paddle by a lot
        //for (std::size_t i = 0, e = _weight_updates.size(); i < e; ++i) {
        //    nn.weights_[0] -= (_weight_updates[i] * LEARNING_RATE * 2.0f);
        //    nn.biases_[0] = _component_subtraction(nn.biases_[0], _component_product(_bias_updates[i], LEARNING_RATE * 2.0f));
        //}

        //nn.weights_[0] -= (_weight_update * LEARNING_RATE * (float)_winner * 10.0f);
        //nn.biases_[0] = _component_subtraction(nn.biases_[0], _component_product(_bias_update, LEARNING_RATE * (float)_winner * 10.0f));
        /*for (std::size_t i = 0, e = _w0_updates.size(); i < e; ++i) {
            nn.weights_[0] -= (_w0_updates[i] * LEARNING_RATE);
            nn.biases_[0] = _component_subtraction(nn.biases_[0], _component_product(_b0_updates[i], LEARNING_RATE));
        }

        for (std::size_t i = 0, e = _w1_updates.size(); i < e; ++i) {
            nn.weights_[1] -= (_w1_updates[i] * LEARNING_RATE);
            nn.biases_[1] = _component_subtraction(nn.biases_[1], _component_product(_b1_updates[i], LEARNING_RATE));
        }*/


        yv += dy_p1;
    }

    if (hit_p2) yv += dy_p2;

    bool bounced = false;

    if (hit_p1 || hit_p2) {
        xv *= -1;

        pong.x = x_valid;
        pong.y = y_valid;
        bounced = true;
    }

    if (pong.y < 0 || pong.y > WINDOW_HEIGHT - pong.h) {
        yv *= -1;

        pong.x = x_valid;
        pong.y = y_valid;
        bounced = true;
    }

    if (!bounced) {
        x_valid = pong.x;
        y_valid = pong.y;
    }



    bool game_over = false;
    if (pong.x < -3) {
        _winner = 1;
        game_over = true;
        ++_ai_winner;
    }

    if (pong.x > WINDOW_WIDTH + 3) {
        _winner = -1;
        game_over = true;
        ++_nn_winner;
    }

    static int loops = 0;
    ++loops;

    // Assume something is stuck and signal game over
    if (loops > 5000) {
        game_over = true;
        _winner = 1;
        ++_ai_winner;
    }

    if (game_over) {
        for (std::size_t i = 0, e = _w0_updates.size(); i < e; ++i) {
            nn.weights_[0] -= (_w0_updates[i] * LEARNING_RATE * (float)_winner * -1.0f);
            nn.biases_[0] = _component_subtraction(nn.biases_[0], _component_product(_b0_updates[i], LEARNING_RATE * (float)_winner * -1.0f));
        }

        for (std::size_t i = 0, e = _w1_updates.size(); i < e; ++i) {
            nn.weights_[1] -= (_w1_updates[i] * LEARNING_RATE * (float)_winner * -1.0f);
            nn.biases_[1] = _component_subtraction(nn.biases_[1], _component_product(_b1_updates[i], LEARNING_RATE * (float)_winner * -1.0f));
        }

        _w0_updates.clear();
        _b0_updates.clear();

        _w1_updates.clear();
        _b1_updates.clear();

        //nn.weights_[0] -= (_weight_update * LEARNING_RATE * (float)_winner * -1.0f);
        //nn.biases_[0] = _component_subtraction(nn.biases_[0], _component_product(_bias_update, LEARNING_RATE * (float)_winner * -1.0f));

        //_weight_update.set(0.0);
        //_bias_update[0] = 0.0;

        //nn.debug_print();

        // Reset everyone
        p1.x = 0; p1.y = WINDOW_HEIGHT / 2 - PADDLE_HEIGHT; p1.w = PADDLE_WIDTH; p1.h = PADDLE_HEIGHT;
        p2.x = WINDOW_WIDTH - PADDLE_WIDTH; p2.y = WINDOW_HEIGHT / 2 - PADDLE_HEIGHT; p2.w = PADDLE_WIDTH; p2.h = PADDLE_HEIGHT;
        pong.x = WINDOW_WIDTH / 2; pong.y = WINDOW_HEIGHT / 2 - PONG_HEIGHT; pong.w = PONG_WIDTH; pong.h = PONG_HEIGHT;
        xv = PONG_SPEED; yv = 0.0;
        std::cout << "Games played: " << ++_gamesplayed << " Winner: " << _winner << " nn_winner: " << _nn_winner << " ai_winner: " << _ai_winner << " ratio: " << (float)_nn_winner / ((float)_ai_winner + _nn_winner)<< std::endl;
        loops = 0;
        return;
    }

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

    NeuralNet<float> nn({ GRID_Y_COUNT * GRID_X_COUNT, 100, 1 });
    //nn.debug_print();

    SDL_Event e;
    int sim_speed = 1;
    bool render = true;
    while (true) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_KEYDOWN: {
                switch (e.key.keysym.sym) {
                case SDLK_1:
                    sim_speed = 1;
                    render = true;
                    break;
                case SDLK_2:
                    sim_speed = 10;
                    render = true;
                    break;
                case SDLK_3:
                    sim_speed = 20;
                    render = true;
                    break;
                case SDLK_4:
                    sim_speed = 30;
                    render = true;
                    break;
                case SDLK_5:
                    sim_speed = 40;
                    render = true;
                    break;
                case SDLK_6:
                    sim_speed = 50;
                    render = false;
                    break;
                }
                break;
            }
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
        if (!render) acc = 1.0;
        for (int i = 0; i < sim_speed; ++i) {
            float acc_recover = acc;
            while (acc >= dt) {
                update_simulation(p1, p2, pong, xv, yv, dt, nn);
                acc -= dt;
                t += dt;
            }
            acc = acc_recover;
        }

        acc = 0.0;
        if (!render) continue;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &p1);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &p2);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &pong);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        int start = 0;
        for (int i = 0; i < GRID_X_COUNT; ++i) {
            SDL_RenderDrawLine(renderer, start, 0, start, WINDOW_HEIGHT);
            start += WIDTH_SIZE;
        }

        start = 0;
        for (int i = 0; i < GRID_Y_COUNT; ++i) {
            SDL_RenderDrawLine(renderer, 0, start, WINDOW_WIDTH, start);
            start += HEIGHT_SIZE;
        }

        SDL_RenderPresent(renderer);
    }

    return 0;
}

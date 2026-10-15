#pragma once
#include <SDL3/SDL.h>

struct InputState {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool turn_left = false;
    bool turn_right = false;
    bool quit = false;
};

inline InputState read_input(const bool* keys) {
    InputState in;
    in.forward = keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
    in.backward = keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
    in.left  = keys[SDL_SCANCODE_A];
    in.right = keys[SDL_SCANCODE_D];
    in.turn_left = keys[SDL_SCANCODE_LEFT];
    in.turn_right = keys[SDL_SCANCODE_RIGHT];
    return in;
}

inline void handle_event(InputState& in, const SDL_Event& e) {
    if (e.type == SDL_EVENT_QUIT) in.quit = true;
    if (e.type == SDL_EVENT_KEY_DOWN &&
        e.key.scancode == SDL_SCANCODE_ESCAPE) in.quit = true;
}

#include "wolf/input/SdlInput.hpp"

#include <SDL3/SDL.h>

namespace wolf {

InputState read_input() {
    // SDL3 timestamps are nanoseconds. Key repeat is not a second press.
    static Uint64 last_w_down = 0;
    static bool double_tap_sprint = false;
    constexpr Uint64 double_tap_window = 280'000'000ULL;

    InputState input;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT ||
            (event.type == SDL_EVENT_KEY_DOWN &&
             event.key.scancode == SDL_SCANCODE_ESCAPE)) {
            input.quit = true;
        }

        if (event.type == SDL_EVENT_KEY_DOWN &&
            event.key.scancode == SDL_SCANCODE_W && !event.key.repeat) {
            const Uint64 now = event.key.timestamp;
            double_tap_sprint = last_w_down != 0 &&
                                now >= last_w_down &&
                                now - last_w_down <= double_tap_window;
            last_w_down = now;
        }

        if (event.type == SDL_EVENT_KEY_UP &&
            event.key.scancode == SDL_SCANCODE_W) {
            double_tap_sprint = false;
        }
    }

    const bool* keys = SDL_GetKeyboardState(nullptr);
    input.moveForward = keys[SDL_SCANCODE_W];
    input.moveBackward = keys[SDL_SCANCODE_S];
    input.moveLeft = keys[SDL_SCANCODE_A];
    input.moveRight = keys[SDL_SCANCODE_D];
    input.turnLeft = keys[SDL_SCANCODE_LEFT];
    input.turnRight = keys[SDL_SCANCODE_RIGHT];
    input.sprint = keys[SDL_SCANCODE_LSHIFT] ||
                   keys[SDL_SCANCODE_RSHIFT] ||
                   (double_tap_sprint && input.moveForward);

    return input;
}

}

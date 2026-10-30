#include "wolf/input/SdlInput.hpp"

#include <SDL3/SDL.h>

namespace wolf {

    InputState read_input() {
        InputState input;
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                input.quit = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN &&
                event.key.scancode == SDL_SCANCODE_ESCAPE) {
                input.quit = true;
                }
        }

        const bool* keys = SDL_GetKeyboardState(nullptr);

        input.moveForward = keys[SDL_SCANCODE_W];
        input.moveBackward = keys[SDL_SCANCODE_S];
        input.moveLeft = keys[SDL_SCANCODE_A];
        input.moveRight = keys[SDL_SCANCODE_D];
        input.turnLeft = keys[SDL_SCANCODE_LEFT];
        input.turnRight = keys[SDL_SCANCODE_RIGHT];

        return input;
    }

}

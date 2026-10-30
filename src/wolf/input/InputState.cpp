#include <input/InputState.h>

void input::InputState::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            const bool active = event.type == SDL_EVENT_KEY_DOWN;

            switch (event.key.scancode) {
                case SDL_SCANCODE_W:
                    setActionActive(Action::MoveForward, active);
                    break;

                case SDL_SCANCODE_S:
                    setActionActive(Action::MoveBackward, active);
                    break;

                case SDL_SCANCODE_A:
                    setActionActive(Action::MoveLeft, active);
                    break;

                case SDL_SCANCODE_D:
                    setActionActive(Action::MoveRight, active);
                    break;

                case SDL_SCANCODE_LEFT:
                    setActionActive(Action::TurnLeft, active);
                    break;

                case SDL_SCANCODE_RIGHT:
                    setActionActive(Action::TurnRight, active);
                    break;

                default:
                    break;
            }

            break;
        }

        case SDL_EVENT_MOUSE_MOTION:
            addMouseDeltaX(event.motion.xrel);
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            reset();
            break;

        default:
            break;
    }
}
#include <stdexcept>
#include <input/InputState.h>

namespace {
    constexpr SDL_Scancode MOVE_FORWARD_KEY = SDL_SCANCODE_W;
    constexpr SDL_Scancode MOVE_BACKWARD_KEY = SDL_SCANCODE_S;
    constexpr SDL_Scancode MOVE_LEFT_KEY = SDL_SCANCODE_A;
    constexpr SDL_Scancode MOVE_RIGHT_KEY = SDL_SCANCODE_D;
}

void input::InputState::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            const bool active = event.type == SDL_EVENT_KEY_DOWN;

            switch (event.key.scancode) {
                case MOVE_FORWARD_KEY:
                    setActionActive(Action::MoveForward, active);
                    break;

                case MOVE_BACKWARD_KEY:
                    setActionActive(Action::MoveBackward, active);
                    break;

                case MOVE_LEFT_KEY:
                    setActionActive(Action::MoveLeft, active);
                    break;

                case MOVE_RIGHT_KEY:
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

double input::InputState::getAxis(MovementAxis axis) const {
    switch (axis) {
        case MovementAxis::Forward:
            return axisValue(Action::MoveBackward, Action::MoveForward);

        case MovementAxis::Horizontal:
            return axisValue(Action::MoveLeft, Action::MoveRight);
    }

    throw std::invalid_argument("Unknown movement axis");
}

double input::InputState::getAxis(RotationAxis axis) const {
    switch (axis) {
        case RotationAxis::Horizontal:
            return axisValue(Action::TurnRight, Action::TurnLeft);
    }

    throw std::invalid_argument("Unknown rotation axis");
}

double input::InputState::axisValue(
    Action negative,
    Action positive
) const {
    return static_cast<double>(isActionActive(positive))
         - static_cast<double>(isActionActive(negative));
}

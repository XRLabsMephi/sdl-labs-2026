#include <debug/Logger.h>
#include <stdexcept>
#include <input/InputState.h>

void input::InputState::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            const bool active = event.type == SDL_EVENT_KEY_DOWN;

            for (std::size_t i = 0; i < bindings.size(); ++i) {
                if (event.key.scancode == bindings[i]) {
                    setActionActive(static_cast<Action>(i), active);
                    break;
                }
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

    throw logging::Error(logging::ErrorCode::InputAxis, "Unknown movement axis");
}

double input::InputState::getAxis(RotationAxis axis) const {
    switch (axis) {
        case RotationAxis::Horizontal:
            return axisValue(Action::TurnRight, Action::TurnLeft);
    }

    throw logging::Error(logging::ErrorCode::InputAxis, "Unknown rotation axis");
}

double input::InputState::axisValue(
    Action negative,
    Action positive
) const {
    return static_cast<double>(isActionActive(positive))
         - static_cast<double>(isActionActive(negative));
}

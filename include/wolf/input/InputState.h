#pragma once
#include <cstddef>
#include <array>
#include <SDL3/SDL_events.h>
namespace input {
    enum class Action {
        MoveForward,
        MoveBackward,
        MoveLeft,
        MoveRight,
        TurnLeft,
        TurnRight,
        Count
    };

    class InputState {
    private:
        static constexpr std::size_t actionCount =
            static_cast<std::size_t>(Action::Count);

        std::array<bool, actionCount> actions{};

        double mouseDeltaX = 0.0;

    public:
        void beginFrame() {
            mouseDeltaX = 0.0;
        }

        void handleEvent(const SDL_Event& event);

        bool isActionActive(Action action) const {
            std::size_t index = static_cast<std::size_t>(action);

            return index < actionCount && actions[index];
        }

        void setActionActive(Action action, bool active) {
            std::size_t index = static_cast<std::size_t>(action);

            if (index < actionCount) {
                actions[index] = active;
            }
        }

        double getMouseDeltaX() const {
            return mouseDeltaX;
        }

        void addMouseDeltaX(double delta) {
            mouseDeltaX += delta;
        }

        void reset() {
            actions.fill(false);
            mouseDeltaX = 0.0;
        }
    };
}

#pragma once
#include "InputState.h"
#include "../core/Vec2.h"
#include "../world/Player.h"
#include "../world/Map.h"
namespace input {
    class PlayerController {
    private:
        double moveSpeed = 3.0;
        double rotationSpeed = 2.0;
        double mouseSensitivity = 0.002;

        core::Vector2 calculateMovement(
            const world::Player& player,
            const InputState& input,
            double deltaTime
        ) const;

        void tryMove(
            world::Player& player,
            const world::Map& map,
            core::Vector2 displacement
        ) const;

        void updateRotation(
            world::Player& player,
            const InputState& input,
            double deltaTime
        ) const;

    public:
        void update(
            world::Player& player,
            const world::Map& map,
            const InputState& input,
            double deltaTime
        ) const;

        double getMoveSpeed() const {
            return moveSpeed;
        }

        void setMoveSpeed(double speed) {
            if (speed >= 0.0) {
                moveSpeed = speed;
            }
        }

        double getRotationSpeed() const {
            return rotationSpeed;
        }

        void setRotationSpeed(double speed) {
            if (speed >= 0.0) {
                rotationSpeed = speed;
            }
        }

        double getMouseSensitivity() const {
            return mouseSensitivity;
        }

        void setMouseSensitivity(double sensitivity) {
            if (sensitivity >= 0.0) {
                mouseSensitivity = sensitivity;
            }
        }
    };
}
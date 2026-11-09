#include "wolf/input/PlayerController.hpp"

#include <cmath>

namespace input {
    PlayerController::PlayerController(double moveSpeed, double turnSpeed, double radius)
        : moveSpeed_(moveSpeed), turnSpeed_(turnSpeed), radius_(radius) {}

    void PlayerController::update(const InputState &input, world::Player &player,
                                  const world::Map &map, double dt) {
        double rotation = 0.0;
        if (input.isSet(InputState::kTurnLeft)) {
            rotation -= turnSpeed_ * dt;
        }
        if (input.isSet(InputState::kTurnRight)) {
            rotation += turnSpeed_ * dt;
        }
        if (rotation != 0.0) {
            player.rotate(rotation);
        }

        core::Vec2 delta{0.0, 0.0};
        core::Vec2 dir = player.direction();
        core::Vec2 plane = player.cameraPlane();

        if (input.isSet(InputState::kForward)) {
            delta += dir;
        }
        if (input.isSet(InputState::kBackward)) {
            delta -= dir;
        }
        if (input.isSet(InputState::kLeft)) {
            delta -= plane.normalized();
        }
        if (input.isSet(InputState::kRight)) {
            delta += plane.normalized();
        }

        if (delta.length() > 0.0) {
            delta = delta.normalized() * (moveSpeed_ * dt);

            core::Vec2 oldPos = player.position();

            core::Vec2 tryX{oldPos.x + delta.x, oldPos.y};
            if (canStand(tryX, map)) {
                player.setPosition(tryX);
            }

            core::Vec2 curPos = player.position();
            core::Vec2 tryY{curPos.x, curPos.y + delta.y};
            if (canStand(tryY, map)) {
                player.setPosition(tryY);
            }
        }
    }

    bool PlayerController::canStand(core::Vec2 pos, const world::Map &map) const {
        double r = radius_;

        int cx_0 = (int)std::floor(pos.x - r);
        int cy_0 = (int)std::floor(pos.y - r);
        int cx_1 = (int)std::floor(pos.x + r);
        int cy_1 = (int)std::floor(pos.y + r);
        
        std::pair<int, int> corners[] = {
            {cx_0, cy_0},
            {cx_0, cy_1},
            {cx_1, cy_0},
            {cx_1, cy_1},
        };

        for (const auto [x, y] : corners) {
            if (map.isSolid(x, y)) {
                return false;
            }
        }
        return true;
    }
}

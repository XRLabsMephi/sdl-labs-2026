#include "wolf/input/PlayerController.hpp"

#include <cmath>
#include <iostream>

namespace input {
    PlayerController::PlayerController(double moveSpeed, double turnSpeed, double radius)
        : moveSpeed_(moveSpeed), turnSpeed_(turnSpeed), radius_(radius) {}

    void PlayerController::update(const InputState &input, world::Player &player,
                                  const world::Map &map, double dt) {

        // Unstuck
        {
            core::Vec2 ppos = player.position();
            if (!canStand(ppos, map)) {
                std::cerr << "Stuck: " << ppos.x << ' ' << ppos.y << std::endl;
                core::Vec2 newPos = findClosestFreePos(ppos, map);
                std::cerr << "New pos: " << newPos.x << ' ' << newPos.y << std::endl;
                player.setPosition(newPos);
                return;
            }
        }

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
        core::Vec2 corners[] = {
            {-r, -r}, {r, -r}, {-r, r}, {r, r}
        };

        for (const auto &c : corners) {
            core::Vec2 check = pos + c;

            int cx = (int) check.x;
            int cy = (int) check.y;

            if (map.isSolid(cx, cy)) {
                return false;
            }
        }
        return true;
    }

    core::Vec2 PlayerController::findClosestFreePos(core::Vec2 pos, const world::Map &map) const {
        size_t w       = map.width();
        size_t h       = map.height();
        double minDist = std::numeric_limits<double>::max();

        core::Vec2 res = pos;
        for (size_t y = 0; y < h; ++y) {
            for (size_t x = 0; x < w; ++x) {
                core::Vec2 cand = {static_cast<double>(x) + 0.5,
                                   static_cast<double>(y) + 0.5};

                if (!canStand(cand, map)) 
                    continue;

                double d = (cand - pos).length();
                
                if (d < minDist) {
                    minDist = d;
                    res = cand;
                }
            }
        }
        return res;
    }
}

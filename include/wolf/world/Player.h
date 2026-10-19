#pragma once
#include "../core/Vec2.h"
namespace world {
    class Player {
    private:
        core::Vector2 position;
        double angle = 0.0;
        double radius = 0.2;

    public:
        core::Vector2 getPosition() const {
            return position;
        }

        void setPosition(core::Vector2 newPosition) {
            position = newPosition;
        }

        double getAngle() const {
            return angle;
        }

        double getRadius() const {
            return radius;
        }

        void rotate(double angleDelta);

        core::Vector2 getDirection() const;
    };
}
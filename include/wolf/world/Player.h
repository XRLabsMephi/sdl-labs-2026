#pragma once

#include <numbers>
#include <core/Vec2.h>

namespace world {

    class Player {
    private:
        core::Vector2 position{};

        double rotation = 0.0;
        double angle = std::numbers::pi / 3.0;
        double radius = 0.2;

    public:
        core::Vector2 getPosition() const {
            return position;
        }

        void setPosition(core::Vector2 newPosition) {
            position = newPosition;
        }

        // Return the field of view in radians.
        double getAngle() const {
            return angle;
        }

        // Return the viewing direction angle in radians.
        double getRotation() const {
            return rotation;
        }

        double getRadius() const {
            return radius;
        }

        void rotate(double angleDelta);

        core::Vector2 getDirection() const;
    };

}
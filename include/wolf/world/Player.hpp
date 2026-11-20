#pragma once

#include <cmath>

#include "wolf/core/Vec2.hpp"

namespace world {
    class Player {
    public:
        Player(core::Vec2 pos, core::Vec2 dir, double fovRadians)
            : position_(pos), direction_(dir.normalized()), fov_(fovRadians) {
            updatePlane();
        }

        [[nodiscard]] core::Vec2 position() const noexcept {
            return position_;
        }

        [[nodiscard]] core::Vec2 direction() const noexcept {
            return direction_;
        }

        [[nodiscard]] core::Vec2 cameraPlane() const noexcept {
            return plane_;
        }

        [[nodiscard]] double fov() const noexcept {
            return fov_;
        }

        void setPosition(core::Vec2 pos) noexcept {
            position_ = pos;
        }

        void move(core::Vec2 delta) noexcept {
            position_ = position_ + delta;
        }

        // Положительный угол поворачивает взгляд вправо.
        void rotate(double angle) {
            direction_ = direction_.rotated(angle);
            plane_ = plane_.rotated(angle);
        }

        void setFov(double fovRadians) {
            fov_ = fovRadians;
            updatePlane();
        }

    private:
        void updatePlane() {
            double len = std::tan(fov_ * 0.5);
            plane_ = direction_.perpendicular() * len;
        }

        core::Vec2 position_{};
        core::Vec2 direction_{1.0, 0.0}; // нормированное
        core::Vec2 plane_{}; // нормированное
        double fov_ = 1.15;
    };
}

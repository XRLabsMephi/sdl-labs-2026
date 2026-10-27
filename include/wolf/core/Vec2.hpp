#pragma once

#include <cmath>

namespace wolf {

    inline constexpr double minVectorLength = 1e-12;

    struct Vec2 {
        double x = 0.0;
        double y = 0.0;

        [[nodiscard]] constexpr Vec2 operator+(Vec2 other) const noexcept {
            return {x + other.x, y + other.y};
        }

        [[nodiscard]] constexpr Vec2 operator*(double factor) const noexcept {
            return {x * factor, y * factor};
        }

        [[nodiscard]] constexpr double length() const {
            return std::hypot(x, y);
        }

        [[nodiscard]] friend double dot(Vec2 a, Vec2 b) {
            return a.x * b.x + a.y * b.y;
        }

        [[nodiscard]] friend inline bool isFinite(Vec2 vector) {
            return std::isfinite(vector.x) && std::isfinite(vector.y);
        }
    };

}

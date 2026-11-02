#pragma once

#include <cmath>

namespace wolf {

struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2 operator+(Vec2 other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(Vec2 other) const { return {x - other.x, y - other.y}; }
    Vec2 operator*(double value) const { return {x * value, y * value}; }

    double length() const { return std::hypot(x, y); }

    Vec2 rotated(double angle) const {
        const double c = std::cos(angle);
        const double s = std::sin(angle);
        return {x * c - y * s, x * s + y * c};
    }
};

}

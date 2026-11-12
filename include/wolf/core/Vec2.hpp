#pragma once

namespace wolf::core {
    struct Vec2 {
        double x = 0.0;
        double y = 0.0;
        
        Vec2 operator+(const Vec2& other) const {
            return {x + other.x, y + other.y};
        }

        Vec2 operator-(const Vec2& other) const {
            return {x - other.x, y - other.y};
        }

        Vec2 operator*(double scalar) const {
            return {x * scalar, y * scalar};
        }
    };
}
#pragma once
#include <cmath>

struct Vector2 {
    double x = 0.0;
    double y = 0.0;

    Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
    Vector2& operator-=(const Vector2& o) { x -= o.x; y -= o.y; return *this; }
    Vector2& operator*=(double s)         { x *= s; y *= s;   return *this; }

    [[nodiscard]] double get_length() const { return std::sqrt(x * x + y * y); }

    void normalize() {
        auto length = get_length();
        if (length == 0) return;
        *this *= 1.0 / length;
    }

    void rotate(double angle) {
        double sin = std::sin(angle);
        double cos = std::cos(angle);
        double rot_x = cos * x + -sin * y;
        double rot_y = sin * x + cos * y;

        x = rot_x;
        y = rot_y;
    }
};

inline Vector2 operator+(Vector2 a, const Vector2& b) { return a += b; }
inline Vector2 operator-(Vector2 a, const Vector2& b) { return a -= b; }
inline Vector2 operator*(Vector2 v, double s)      { return v *= s; }
inline Vector2 operator*(double s, Vector2 v)      { return v *= s; }
#pragma once
#include <cmath>

namespace core {
    struct Vector2 {
        double x = 0.0, y = 0.0;

        Vector2 operator-(const Vector2& other) const{
            return {x - other.x, y - other.y};
        }
        Vector2 operator+(const Vector2& other) const
        {
            return {x + other.x, y + other.y};
        }
        Vector2& operator-=(const Vector2& other){
            x -= other.x;
            y -= other.y;
            return *this;
        }
        Vector2& operator+=(const Vector2& other){
            x += other.x;
            y += other.y;
            return *this;
        }
    };
    inline double distance(const Vector2& v1, const Vector2& v2){
        return sqrt((v1.x - v2.x)*(v1.x-v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
    }
    inline double length(const Vector2& v){
        return sqrt(v.x * v.x + v.y * v.y);
    }
}

#pragma once
#include <cmath>

namespace core {
    struct Vector2 {
        float x = 0.0f, y = 0.0f;

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
    inline float distance(const Vector2& v1, const Vector2& v2){
        return sqrtf((v1.x - v2.x)*(v1.x-v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
    }
    inline float length(const Vector2& v){
        return sqrtf(v.x * v.x + v.y * v.y);
    }
}

#pragma once
#include "wolf/core/Color.hpp"
#include <vector>

struct Texture {
    int width = 0;
    int height = 0;
    std::vector<Color> pixels;
};
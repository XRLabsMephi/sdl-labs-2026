#pragma once

#include <vector>

#include "wolf/core/Color.hpp"

namespace wolf {

    struct Texture {
        int width = 0;
        int height = 0;
        std::vector<Color> pixels;

        Color sample(int x, int y) const;
    };

}

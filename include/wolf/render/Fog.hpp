#pragma once

#include "wolf/core/Color.hpp"

namespace wolf {

struct Fog {
    double start = 5.0;
    double end = 15.0;
    Color color{100, 100, 100, 255};

    Color apply(Color source, double depth) const;
};

}

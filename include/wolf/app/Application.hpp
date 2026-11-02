#pragma once

#include "wolf/input/InputState.hpp"

namespace wolf {

class Application {
public:
    int run();

private:
    InputState readInput() const;
};

}

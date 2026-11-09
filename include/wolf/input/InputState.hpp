#pragma once

namespace wolf
{
struct InputState
{
    bool forward = false;
    bool backward = false;
    bool strafeLeft = false;
    bool strafeRight = false;
    bool turnLeft = false;
    bool turnRight = false;
    bool quit = false;
};
}

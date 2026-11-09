#pragma once

namespace wolf {

    struct InputState {
        bool moveForward = false;
        bool moveBackward = false;
        bool moveLeft = false;
        bool moveRight = false;
        bool sprint = false;
        bool drift = false;
        bool dash = false;
        bool restartRun = false;
        bool turnLeft = false;
        bool turnRight = false;
        bool quit = false;
    };

}

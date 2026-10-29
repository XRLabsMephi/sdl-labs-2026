#pragma once

namespace wolf {

    struct InputState {
        bool moveForward = false;
        bool moveBackward = false;
        bool moveLeft = false;
        bool moveRight = false;
        bool turnLeft = false;
        bool turnRight = false;
        bool quit = false;
    };

}

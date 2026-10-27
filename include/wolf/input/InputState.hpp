#pragma once

namespace wolf {

    struct InputState {
        bool moveForward = false;
        bool moveBackward = false;
        bool strafeLeft = false;
        bool strafeRight = false;
        bool turnLeft = false;
        bool turnRight = false;
        bool quit = false;
    };

}

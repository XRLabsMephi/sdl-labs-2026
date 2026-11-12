#pragma once
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/raycast/RayHit.hpp"

namespace wolf::raycast {
    class Raycaster {
     public:
        void render(render::FrameBuffer& fb, const world::Player& player, const world::Map& map);
     private:
        RayHit DDA_Ray(const world::Player& player, const world::Map& map, double cameraX);
    };

}
#pragma once

#include <vector>

#include "wolf/raycast/RayHit.hpp"
#include "wolf/render/framebuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace raycast {
    class Raycaster {
    public:
        Raycaster() = default;

        // Renders walls into framebuffer.
        void render(const world::Player& player,
                    const world::Map&    map,
                    render::Framebuffer& framebuffer);

        // for x in [-1, 1]: (cast 1 ray)
        [[nodiscard]] RayHit castRay(const world::Player& player,
                                     const world::Map&    map,
                                     double               cameraX) const;
    };
}
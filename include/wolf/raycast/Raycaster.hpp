#pragma once

#include "wolf/core/Vec2.hpp"
#include "wolf/raycast/RayHit.hpp"

#include <vector>

namespace wolf {

class Map;
class FrameBuffer;
class TextureStore;
struct Player;

class Raycaster {
public:
    RayHit castRay(const Map& map, Vec2 origin, Vec2 direction) const;
    void render(const Map& map, const Player& player, FrameBuffer& frame,
                const TextureStore* textures = nullptr);
    const std::vector<double>& depthBuffer() const;

private:
    std::vector<double> depthBuffer_;
};

}

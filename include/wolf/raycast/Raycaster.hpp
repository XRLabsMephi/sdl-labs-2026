#pragma once

#include "wolf/raycast/RayHit.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <vector>

namespace wolf
{
class Raycaster
{
public:
    void render(const Map& map, const Player& player, FrameBuffer& frameBuffer);
    const std::vector<double>& depthBuffer() const;

private:
    RayHit castRay(const Map& map, const Player& player, int screenX, int screenWidth) const;
    Color wallColor(int wallId, int side) const;

    std::vector<double> depthBuffer_;
};
}

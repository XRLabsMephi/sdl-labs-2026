#pragma once

#include "wolf/raycast/RayHit.hpp"

namespace wolf
{

class FrameBuffer;
class Map;
struct Player;

class Raycaster
{
public:
    [[nodiscard]] RayHit cast(const Map& map, const Player& player, double cameraX) const;
    void render(const Map& map, const Player& player, FrameBuffer& frameBuffer) const;
};

} // namespace wolf

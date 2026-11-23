#pragma once

#include "wolf/raycast/RayHit.hpp"

#include <span>

namespace wolf
{

class FrameBuffer;
class Map;
class TextureStore;
struct Fog;
struct Player;

class Raycaster
{
public:
    [[nodiscard]] RayHit cast(const Map& map, const Player& player, double cameraX) const;
    void render(const Map& map, const Player& player, FrameBuffer& frameBuffer) const;
    void renderTextured(const Map& map, const Player& player, const TextureStore& textures,
                        const Fog& fog, FrameBuffer& frame, std::span<double> wallDepths) const;
};

} // namespace wolf

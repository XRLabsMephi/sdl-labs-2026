#pragma once

#include "wolf/world/Sprite.hpp"

#include <span>

namespace wolf
{

class FrameBuffer;
class TextureStore;
struct Fog;
struct Player;

class SpriteRenderer
{
public:
    void render(std::span<const Sprite> sprites, const Player& player,
                const TextureStore& textures, std::span<const double> wallDepths,
                const Fog& fog, FrameBuffer& frame) const;
};

} // namespace wolf

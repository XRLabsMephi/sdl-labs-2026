#pragma once

#include "wolf/sprite/Sprite.hpp"

#include <span>

namespace wolf {

struct Player;
class TextureStore;
class FrameBuffer;

class SpriteRenderer {
public:
    void render(std::span<const Sprite> sprites, const Player& player,
                const TextureStore& textures, std::span<const double> wallDepth,
                FrameBuffer& frame) const;
};

}

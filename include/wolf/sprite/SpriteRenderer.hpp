#pragma once

#include "wolf/sprite/Sprite.hpp"

#include <span>

namespace wolf {

struct Player;
struct Fog;
class TextureStore;
class FrameBuffer;

class SpriteRenderer {
public:
    void render(std::span<const Sprite> sprites, const Player& player,
                const TextureStore& textures, std::span<const double> wallDepth,
                FrameBuffer& frame, const Fog* fog = nullptr) const;
};

}

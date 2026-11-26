#pragma once

#include <vector>
#include "wolf/sprite/Sprite.hpp"

namespace wolf {

class FrameBuffer;
struct Player;
class TextureStore;
struct Fog;

class SpriteRenderer {
public:
    void render(FrameBuffer& frame, const std::vector<Sprite>& sprites,
                const Player& player, const TextureStore& textures,
                const std::vector<double>& depthBuffer, const Fog& fog) const;
};

}

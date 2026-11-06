#pragma once

#include "wolf/texture/TextureStore.hpp"
#include <vector>

struct Sprite;
struct Player;
class FrameBuffer;

class SpriteRenderer {
public:
    void render(const std::vector<Sprite>& sprites, const Player& player, FrameBuffer& frame, const TextureStore& textures, const std::vector<double>& depthBuffer) const;
};
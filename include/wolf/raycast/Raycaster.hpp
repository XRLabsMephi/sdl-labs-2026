#pragma once

#include "wolf/core/Vec2.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/raycast/RayHit.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/effects/Fog.hpp"
#include <vector>

class Map;
class FrameBuffer;

class Raycaster {
public:
    Vec2 rayDirection(const Player& player, double cameraX) const;
    RayHit castRay(const Map& map, const Player& player, double cameraX) const;
    void render(const Map& map, const Player& player, FrameBuffer& frame, const TextureStore& textures = {}, const Fog& fog = {});
    const std::vector<double>& depthBuffer() const;

private:
    std::vector<double> depthBuffer_;
};
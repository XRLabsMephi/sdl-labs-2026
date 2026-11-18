#pragma once
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/raycast/RayHit.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/effects/Fog.hpp"

namespace wolf::raycast {
    class Raycaster {
     public:
        void render(render::FrameBuffer& fb, const world::Player& player, const world::Map& map, const texture::TextureStore& textureStore, const effects::Fog& fog);
        [[nodiscard]] const std::vector<double>& getDepthBuffer() const noexcept;
     private:
        RayHit DDA_Ray(const world::Player& player, const world::Map& map, double cameraX);
        std::vector<double> depthBuffer;
    };

}
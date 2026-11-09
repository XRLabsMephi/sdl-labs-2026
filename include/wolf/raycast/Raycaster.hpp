#pragma once

#include <cstdint>
#include <vector>

#include "wolf/raycast/RayHit.hpp"
#include "wolf/render/framebuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"
// #include "wolf/texture/TextureStore.hpp"
// #include "wolf/effects/Fog.hpp"

namespace raycast {
    class Raycaster {
    public:
        Raycaster() = default;

        // Renders walls into framebuffer.
        void render(const world::Player&         player,
                    const world::Map&            map,
                    const texture::TextureStore& textures,
                    const effects::Fog&          fog,
                    render::Framebuffer&         framebuffer) noexcept;

        // depth buffer getter
        [[nodiscard]] const std::vector<double>& depthBuffer() const noexcept {
            return depthBuffer_;
        }

        // for x in [-1, 1]: (cast 1 ray)
        void castRay(const world::Player& player,
                     const world::Map&    map,
                     double               cameraX) noexcept;

    private:
        // different context for each frame, same for columns
        struct FrameCtx {
            uint32_t*           pixels = nullptr;
            uint32_t            w      = 0;
            uint32_t            h      = 0;
            const effects::Fog& fog;
        };

        // different context for each column
        struct ColumnCtx {
            uint32_t x          = 0;
            uint32_t drawStart  = 0;
            uint32_t drawEnd    = 0;
            double   lineHeight = 0.0;
        };

        void drawTexturedColumn(const FrameCtx&  f,
                                const ColumnCtx& c,
                                const RayHit&    rh,
                                const texture::Texture& tex) noexcept;

        void drawSolidColumn(const FrameCtx&  f,
                             const ColumnCtx& c,
                             const RayHit&    rh) noexcept;

        // buffers
        std::vector<RayHit> rayHits_;
        std::vector<double> depthBuffer_;
    };
}
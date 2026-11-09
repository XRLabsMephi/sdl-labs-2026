#pragma once

#include <cstdint>
#include <vector>

#include "wolf/raycast/RayHit.hpp"
#include "wolf/render/framebuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/texture/TextureStore.hpp"

namespace raycast {
    class Raycaster {
    public:
        Raycaster() = default;

        // Renders walls into framebuffer.
        void render(const world::Player&         player,
                    const world::Map&            map,
                    const texture::TextureStore& textures,
                    render::Framebuffer&         framebuffer);

        // Nearest fully opaque wall per column. Alpha-cutout walls are tested per pixel.
        [[nodiscard]] const std::vector<double>& depthBuffer() const noexcept {
            return depthBuffer_;
        }

        // for x in [-1, 1]: (cast 1 ray)
        void castRay(const world::Player& player,
                     const world::Map&    map,
                     double               cameraX,
                     const texture::TextureStore* textures = nullptr);

    private:
        // different context for each frame, same for columns
        struct FrameCtx {
            render::Framebuffer& framebuffer;
            uint32_t            h      = 0;
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
                                const texture::Texture& tex);

        void drawSolidColumn(const FrameCtx&  f,
                             const ColumnCtx& c,
                             const RayHit&    rh);

        // buffers
        std::vector<RayHit> rayHits_;
        std::vector<double> depthBuffer_;
    };
}

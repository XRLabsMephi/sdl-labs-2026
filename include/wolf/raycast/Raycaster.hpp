#pragma once

#include <cstdint>
#include <vector>

#include "wolf/raycast/RayHit.hpp"
#include "wolf/render/framebuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/texture/TextureStore.hpp"

namespace effects { class Fog; }

namespace raycast {
    class Raycaster {
    public:
        Raycaster() = default;

        // Hands the raycaster the scene's fog. drawSolidFogColumn() and the
        // depth check read color() / end() from it, so the fog parameters are
        // never duplicated here. Passing nullptr disables the shortcut.
        // Lifetime: fog must outlive the raycaster.
        void setFog(const effects::Fog* fog) noexcept;

        // Renders walls into framebuffer.
        void render(const world::Player&         player,
                    const world::Map&            map,
                    const texture::TextureStore& textures,
                    render::Framebuffer&         framebuffer) noexcept;

        // Nearest fully opaque wall per column. Alpha-cutout walls are tested per pixel.
        [[nodiscard]] const std::vector<double>& depthBuffer() const noexcept {
            return depthBuffer_;
        }

        // for x in [-1, 1]: (cast 1 ray)
        void castRay(const world::Player& player,
                     const world::Map&    map,
                     const double         cameraX,
                     const texture::TextureStore* textures = nullptr) noexcept;

    private:
        // different context for each frame, same for columns
        struct FrameCtx {
            render::Framebuffer& framebuffer;
            uint32_t h = 0;
            uint32_t w = 0;
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
                                const texture::Texture& tex) const noexcept;

        void drawSolidColumn(const FrameCtx&  f,
                             const ColumnCtx& c,
                             const RayHit&    rh) const noexcept;

        void drawSolidFogColumn(const FrameCtx&  f,
                                const ColumnCtx& c,
                                const RayHit&    rh) const noexcept;

        // buffers
        std::vector<RayHit> rayHits_;
        std::vector<double> depthBuffer_;

        // fog
        const effects::Fog* fog_ = nullptr;
        uint32_t fogColor_ = 0;
    };
}

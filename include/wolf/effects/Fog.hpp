#pragma once

#include "wolf/core/color.hpp"
#include "wolf/render/framebuffer.hpp"

namespace effects {
    class Fog {
    public:
        Fog(double fogStart = 3.0, double fogEnd = 12.0,
            core::ColorRGBA fogColor = {100, 110, 125});

        [[nodiscard]] double factor(double depth) const noexcept;
        [[nodiscard]] core::ColorRGBA apply(core::ColorRGBA color, double depth) const noexcept;

        // Read-only accessors used by the raycaster's fully-fogged wall
        // shortcut. They let the raycaster read the actual fog values without
        // duplicating them in its own translation unit.
        [[nodiscard]] const core::ColorRGBA& color() const noexcept { return color_; }
        [[nodiscard]] double end() const noexcept { return end_; }

        // Final CPU fog/composition pass. Every transparent wall layer is fogged at
        // its own depth, then blended over the fogged opaque scene, far to near.
        // Call exactly once after walls and sprites. Infinite background is preserved.
        void apply(render::Framebuffer& framebuffer) const;

    private:
        double start_;
        double end_;
        core::ColorRGBA color_;
    };
}

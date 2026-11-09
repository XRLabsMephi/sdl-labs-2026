#include "wolf/effects/Fog.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace effects {
    Fog::Fog(double fogStart, double fogEnd, core::ColorRGBA fogColor)
        : start_(fogStart), end_(fogEnd), color_(fogColor) {
        if (!std::isfinite(start_) || !std::isfinite(end_) || start_ < 0.0 || end_ <= start_) {
            throw std::invalid_argument("Fog requires finite 0 <= fogStart < fogEnd");
        }
    }

    double Fog::factor(double depth) const noexcept {
        if (std::isnan(depth)) return 0.0;
        return std::clamp((depth - start_) / (end_ - start_), 0.0, 1.0);
    }

    core::ColorRGBA Fog::apply(core::ColorRGBA color, double depth) const noexcept {
        const double amount = factor(depth);
        if (amount <= 0.0) return color;
        if (amount >= 1.0) return {color_.r, color_.g, color_.b, color.a};
        const auto blend = [amount](std::uint8_t scene, std::uint8_t fog) {
            return static_cast<std::uint8_t>(std::lround(scene * (1.0 - amount) + fog * amount));
        };
        return {blend(color.r, color_.r), blend(color.g, color_.g), blend(color.b, color_.b), color.a};
    }

    void Fog::apply(render::Framebuffer& framebuffer) const {
        auto* pixels = framebuffer.color();
        const auto* depths = framebuffer.depth();
        const auto size = static_cast<std::size_t>(framebuffer.width()) * framebuffer.height();
        const auto fragments = framebuffer.transparentFragments();
        const auto* heads = framebuffer.transparentHeads();
        std::vector<const render::Framebuffer::TransparentFragment*> layers;
        const auto unpack = [](std::uint32_t packed) {
            return core::ColorRGBA{
                static_cast<std::uint8_t>(packed), static_cast<std::uint8_t>(packed >> 8U),
                static_cast<std::uint8_t>(packed >> 16U), static_cast<std::uint8_t>(packed >> 24U)};
        };
        for (std::size_t i = 0; i < size; ++i) {
            if (!std::isfinite(depths[i]) && heads[i] == render::Framebuffer::kNoFragment) continue;
            auto color = unpack(pixels[i]);
            if (std::isfinite(depths[i])) color = apply(color, depths[i]);
            layers.clear();
            for (auto head = heads[i]; head != render::Framebuffer::kNoFragment; head = fragments[head].next) {
                if (fragments[head].depth < depths[i]) layers.push_back(&fragments[head]);
            }
            // Wall fragments and sprites may arrive in different passes: resolve
            // visibility only after the nearest opaque pixel is known.
            // std::sort needs no temporary heap allocation for each screen pixel.
            std::sort(layers.begin(), layers.end(), [](const auto* a, const auto* b) {
                return a->depth > b->depth || (a->depth == b->depth && a < b);
            });
            for (const auto* layer : layers) {
                const auto source = apply(unpack(layer->color), layer->depth);
                const auto blend = [alpha = source.a](std::uint8_t back, std::uint8_t front) {
                    return static_cast<std::uint8_t>((front * alpha + back * (255U - alpha) + 127U) / 255U);
                };
                color = {blend(color.r, source.r), blend(color.g, source.g), blend(color.b, source.b), 255};
            }
            pixels[i] = color.pack();
        }
    }
}

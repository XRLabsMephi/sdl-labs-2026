#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <numbers>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>

#include "wolf/effects/Fog.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"
#include "wolf/texture/TextureStore.hpp"

namespace {
    constexpr auto red = core::ColorRGBA::packColor(240, 20, 10);
    constexpr auto blue = core::ColorRGBA::packColor(10, 20, 240);
    constexpr auto green = core::ColorRGBA::packColor(20, 160, 30);
    constexpr double infinity = std::numeric_limits<double>::infinity();

    void check(bool condition, const std::string& message,
               std::source_location location = std::source_location::current()) {
        if (!condition) {
            throw std::runtime_error(std::to_string(location.line()) + ": " + message);
        }
    }

    template<class Callable>
    void checkThrows(Callable action, const std::string& message) {
        bool threw = false;
        try { action(); } catch (const std::exception&) { threw = true; }
        check(threw, message);
    }

    texture::Texture solid(std::uint32_t color) { return {1, 1, {color}}; }

    world::Player camera() {
        return {{0.0, 0.0}, {1.0, 0.0}, std::numbers::pi / 2.0};
    }

    void textureAndPng() {
        const texture::Texture image{2, 2, {red, blue, green, 0}};
        check(image.sample(0, 0) == red && image.sample(1, 0) == blue, "RGBA texel order");
        check(image.sample(-10, 100) == green && image.sample(100, -10) == blue, "clamped sampling");
        check(texture::Texture{}.sample(0, 0) == 0, "empty texture is transparent");
        check(texture::Texture{2, 2, {red}}.sample(1, 1) == 0, "malformed texture is safe");

        texture::TextureStore textures;
        const std::filesystem::path root = LAB02_FIXTURES_DIR;
        textures.load(1, root / "rgba.png");
        const auto* rgba = textures.get(1);
        check(rgba && rgba->width == 2 && rgba->height == 2, "PNG dimensions");
        check(rgba->sample(0, 0) == core::ColorRGBA(7, 11, 13).pack(), "PNG channels");
        check(rgba->sample(1, 0) >> 24U == 0, "PNG transparent texel");
        check(rgba->sample(0, 1) == core::ColorRGBA(255, 128, 3, 64).pack(), "PNG preserves alpha");
        textures.load(2, root / "rgb.png");
        textures.load(3, root / "gray.png");
        check(textures.get(2)->sample(0, 0) == core::ColorRGBA(11, 21, 31).pack(), "RGB to RGBA");
        check(textures.get(3)->sample(0, 0) == core::ColorRGBA(87, 87, 87).pack(), "grayscale to RGBA");
        checkThrows([&] { textures.load(1, root / "not_png.txt"); }, "reject non-PNG");
        checkThrows([&] { textures.load(1, root / "truncated.png"); }, "reject truncated PNG");
        checkThrows([&] { textures.load(1, root / "missing.png"); }, "report missing PNG");
        check(textures.get(1)->sample(0, 0) == core::ColorRGBA(7, 11, 13).pack(), "failed load retains resource");
        check(textures.get(100) == nullptr, "missing texture lookup");
        checkThrows([&] { textures.set(1, {}); }, "reject malformed resource");
        check(!textures.isOpaque(1) && textures.isOpaque(2) && textures.isOpaque(100), "cached material coverage");
        textures.set(4, solid(red), 128);
        check((textures.get(4)->sample(0, 0) >> 24U) == 128 && !textures.isOpaque(4), "material opacity");
        textures.load(5, root / "rgba.png", 128);
        check((textures.get(5)->sample(0, 1) >> 24U) == 32, "PNG alpha multiplied by material opacity");
        textures.set(4, solid(red));
        check(textures.isOpaque(4), "replacement recomputes opacity cache");
    }

    void fogAndDepth() {
        effects::Fog fog(2.0, 6.0, {100, 120, 140});
        const core::ColorRGBA source(200, 40, 60, 73);
        check(fog.apply(source, 1.0).pack() == source.pack(), "no fog before start");
        check(fog.apply(source, 2.0).pack() == source.pack(), "fog start boundary");
        check(fog.apply(source, 4.0).pack() == core::ColorRGBA(150, 80, 100, 73).pack(), "linear fog midpoint");
        check(fog.apply(source, 6.0).pack() == core::ColorRGBA(100, 120, 140, 73).pack(), "fog end boundary");
        check(fog.factor(100.0) == 1.0, "fog saturates");
        checkThrows([] { effects::Fog invalid(3.0, 3.0); }, "reject empty fog interval");
        checkThrows([] { effects::Fog invalid(0.0, infinity); }, "reject infinite fog interval");

        render::Framebuffer frame(3, 1);
        frame.clear(source);
        check(std::isinf(frame.getDepth(0, 0)), "initial depth is infinite");
        frame.setDepth(0, 0, 4.0f);
        frame.setDepth(1, 0, 8.0f);
        fog.apply(frame);
        check(frame.getPixel(0, 0) == core::ColorRGBA(150, 80, 100, 73).pack(), "per-pixel fog depth");
        check(frame.getPixel(1, 0) == core::ColorRGBA(100, 120, 140, 73).pack(), "far pixel fog");
        check(frame.getPixel(2, 0) == source.pack(), "background retains its color");
        frame.clear();
        check(std::isinf(frame.getDepth(0, 0)), "clear resets depth every frame");
    }

    void spritesAndOcclusion() {
        texture::TextureStore textures;
        textures.set(1, {2, 2, {0, red, red, red}});
        textures.set(2, solid(blue));
        textures.set(3, solid(red));
        sprite::SpriteRenderer renderer;
        render::Framebuffer frame(64, 64);
        std::vector<double> walls(64, 10.0);
        const std::array sprites{sprite::Sprite{{2.0, 0.0}, 1}, sprite::Sprite{{4.0, 0.0}, 2}};
        frame.clear({20, 160, 30});
        renderer.render(camera(), sprites, textures, walls, frame);
        check(frame.getPixel(26, 26) == blue && frame.getDepth(26, 26) == 4.0f,
              "transparent hole preserves farther sprite and its depth");
        check(frame.getPixel(38, 26) == red && frame.getDepth(38, 26) == 2.0f,
              "near sprite covers far sprite, regardless of input order");
        check(frame.getPixel(2, 2) == green && std::isinf(frame.getDepth(2, 2)), "outside sprite unchanged");
        check(frame.getPixel(16, 47) == red && frame.getPixel(16, 48) == green, "floor-aligned projection bounds");

        const std::array opaque{sprite::Sprite{{2.0, 0.0}, 3}};
        std::fill(walls.begin(), walls.begin() + 32, 1.0);
        frame.clear({20, 160, 30});
        renderer.render(camera(), opaque, textures, walls, frame);
        check(frame.getPixel(26, 26) == green && frame.getPixel(38, 26) == red, "partial wall occlusion");
        std::fill(walls.begin(), walls.end(), 2.0);
        frame.clear({20, 160, 30});
        renderer.render(camera(), opaque, textures, walls, frame);
        check(frame.getPixel(38, 26) == green, "sprite at wall depth is occluded");
        checkThrows([&] { renderer.render(camera(), opaque, textures, {}, frame); }, "reject mismatched depth buffer");

        // Radial sorting would put the farther blue sprite over the red one here.
        render::Framebuffer wide(200, 100);
        const std::array offset{sprite::Sprite{{2.0, 2.5}, 3, 4.0}, sprite::Sprite{{3.0, 0.0}, 2, 2.0}};
        renderer.render(camera(), offset, textures, std::vector<double>(200, infinity), wide);
        check(wide.getPixel(130, 30) == red, "sort by camera depth, not Euclidean distance");

        const std::array invalid{
            sprite::Sprite{{-2.0, 0.0}, 3}, sprite::Sprite{{0.0, 0.0}, 3},
            sprite::Sprite{{2.0, 100.0}, 3}, sprite::Sprite{{2.0, 0.0}, 999},
            sprite::Sprite{{2.0, 0.0}, 3, 0.0}, sprite::Sprite{{infinity, 0.0}, 3},
            sprite::Sprite{{2.0, 0.0}, 3, std::numeric_limits<double>::quiet_NaN()}};
        frame.clear({20, 160, 30});
        renderer.render(camera(), invalid, textures, walls, frame);
        check(std::all_of(frame.color(), frame.color() + 64 * 64,
                         [](auto color) { return color == green; }), "invalid / invisible sprites are skipped");

        textures.set(4, {1, 2, {red, blue}});
        const std::array nonSquare{sprite::Sprite{{2.0, 0.0}, 4}};
        std::fill(walls.begin(), walls.end(), infinity);
        renderer.render(camera(), nonSquare, textures, walls, frame);
        check(frame.getPixel(23, 20) == green && frame.getPixel(24, 20) == red &&
              frame.getPixel(39, 40) == blue && frame.getPixel(40, 40) == green, "PNG aspect ratio and both texel rows");

        frame.clear({20, 160, 30});
        const world::Player rotated({3.0, 4.0}, {0.0, 1.0}, std::numbers::pi / 3.0);
        const std::array beside{sprite::Sprite{{2.5, 6.0}, 3}};
        renderer.render(rotated, beside, textures, walls, frame);
        check(frame.getPixel(45, 32) == red && frame.getPixel(10, 32) == green &&
              frame.getDepth(45, 32) == 2.0f, "translated / rotated camera and changed FOV");

        const std::array close{sprite::Sprite{{0.0001, 0.0}, 3}};
        renderer.render(camera(), close, textures, walls, frame);
        check(frame.getPixel(0, 0) == red && frame.getPixel(63, 63) == red, "near sprite is safely clipped");

        frame.clear({20, 160, 30});
        for (int y = 0; y < 64; ++y) for (int x = 0; x < 64; ++x) frame.setDepth(x, y, 8.0f);
        renderer.render(camera(), sprites, textures, walls, frame);
        effects::Fog fog(2.0, 6.0, {100, 110, 120});
        fog.apply(frame);
        check(frame.getPixel(38, 26) == red, "near sprite uses its own fog depth");
        check(frame.getPixel(26, 26) == fog.apply({10, 20, 240}, 4.0).pack(), "far sprite visible through alpha gets its own fog");
        check(frame.getPixel(0, 0) == core::ColorRGBA(100, 110, 120).pack(), "wall behind sprites uses wall fog depth");
    }

    void wallIntegration() {
        const world::Map map({{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1},
                              {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1}});
        world::Player player({2.5, 2.5}, {1.0, 0.0}, std::numbers::pi / 2.0);
        render::Framebuffer frame(64, 64);
        texture::TextureStore textures;
        textures.set(1, solid(red));
        textures.set(2, solid(blue));
        raycast::Raycaster raycaster;
        frame.clear();
        raycaster.render(player, map, textures, frame);
        check(std::abs(raycaster.depthBuffer()[32] - 1.5) < 1e-9, "wall perpendicular depth");
        check(frame.getPixel(32, 32) == red && frame.getDepth(32, 32) == 1.5f, "wall writes color and pixel depth");
        const std::array items{sprite::Sprite{{3.5, 2.5}, 2}};
        sprite::SpriteRenderer renderer;
        renderer.render(player, items, textures, raycaster.depthBuffer(), frame);
        check(frame.getPixel(32, 32) == blue && frame.getDepth(32, 32) == 1.0f, "real wall depth feeds sprite pass");

        // At depth 0.25 the screen covers the middle quarter of the texture,
        // rather than stretching half of it to fill the viewport.
        textures.set(1, {1, 4, {red, blue, green, 0xFFFFFFFFU}});
        player.setPosition({3.75, 2.5});
        frame.clear();
        raycaster.render(player, map, textures, frame);
        check(frame.getPixel(32, 0) == blue && frame.getPixel(32, 63) == green, "near-wall texture clipping");
        check(frame.getDepth(32, 0) == 0.25f, "near wall depth across viewport");
    }

    void transparentLayers() {
        render::Framebuffer frame(1, 1);
        const effects::Fog clearFog(100.0, 200.0);
        frame.drawFragment(0, 0, {0, 0, 240}, 10.0);
        // Insert near before far: final composition must sort independently of insertion order.
        frame.drawFragment(0, 0, {0, 240, 0, 128}, 3.0);
        frame.drawFragment(0, 0, {240, 0, 0, 128}, 6.0);
        clearFog.apply(frame);
        check(frame.getPixel(0, 0) == core::ColorRGBA(60, 120, 60).pack(), "two glass layers, far to near");
        check(frame.getDepth(0, 0) == 10.0f, "glass retains separate depth from opaque surface");

        frame.clear();
        check(frame.transparentFragments().empty() && frame.transparentHeads()[0] == render::Framebuffer::kNoFragment,
              "clear removes all transparent layers");
        frame.drawFragment(0, 0, {0, 0, 240}, 10.0);
        frame.drawFragment(0, 0, {240, 0, 0, 128}, 2.0);
        const effects::Fog blackFog(2.0, 10.0, {0, 0, 0});
        blackFog.apply(frame);
        check(frame.getPixel(0, 0) == core::ColorRGBA(120, 0, 0).pack(), "fog applied to each layer at its own depth");

        frame.clear({10, 20, 30});
        frame.drawFragment(0, 0, {240, 0, 0, 128}, 4.0);
        frame.drawFragment(0, 0, {0, 240, 0}, 2.0);
        clearFog.apply(frame);
        check(frame.getPixel(0, 0) == core::ColorRGBA(0, 240, 0).pack(), "opaque sprite hides glass behind it");

        frame.clear({10, 20, 30});
        frame.drawFragment(0, 0, {240, 0, 0, 0}, 2.0);
        clearFog.apply(frame);
        check(frame.getPixel(0, 0) == core::ColorRGBA(10, 20, 30).pack() && std::isinf(frame.getDepth(0, 0)),
              "zero-alpha wall changes neither background nor depth");
        frame.drawFragment(0, 0, {240, 0, 0, 128}, 2.0);
        clearFog.apply(frame);
        check(frame.getPixel(0, 0) == core::ColorRGBA(125, 10, 15).pack(), "glass over unprojected background");
    }

    void transparentWallIntegration() {
        const world::Map map({{1, 1, 1, 1, 1, 1, 1, 1},
                              {1, 0, 0, 0, 0, 0, 0, 1},
                              {1, 0, 0, 2, 0, 3, 0, 1},
                              {1, 0, 0, 0, 0, 0, 0, 1},
                              {1, 1, 1, 1, 1, 1, 1, 1}});
        const world::Player player({1.5, 2.5}, {1, 0}, std::numbers::pi / 2.0);
        const auto opaqueBlue = core::ColorRGBA(0, 0, 240).pack();
        texture::TextureStore textures;
        textures.set(1, solid(opaqueBlue));
        textures.set(2, {1, 1, {core::ColorRGBA(240, 0, 0, 128).pack()}});
        textures.set(3, {1, 1, {core::ColorRGBA(0, 240, 0, 128).pack()}});
        textures.set(101, solid(opaqueBlue));
        render::Framebuffer frame(64, 64);
        raycast::Raycaster walls;
        sprite::SpriteRenderer sprites;
        const effects::Fog clearFog(100.0, 200.0);
        walls.render(player, map, textures, frame);
        check(std::abs(walls.depthBuffer()[32] - 5.5) < 1e-9, "DDA continues through both transparent walls");
        check(frame.getDepth(32, 32) == 5.5f, "opaque wall remains base behind two glass layers");
        clearFog.apply(frame);
        check(frame.getPixel(32, 32) == core::ColorRGBA(120, 60, 60).pack(), "real DDA glass composition order");

        const auto renderSpriteAt = [&](double x) {
            frame.clear();
            walls.render(player, map, textures, frame);
            const std::array items{sprite::Sprite{{x, 2.5}, 101}};
            sprites.render(player, items, textures, walls.depthBuffer(), frame);
            clearFog.apply(frame);
        };
        renderSpriteAt(2.5);
        check(frame.getPixel(32, 32) == opaqueBlue, "sprite in front of both glass layers");
        renderSpriteAt(4.5);
        check(frame.getPixel(32, 32) == core::ColorRGBA(120, 0, 120).pack(), "sprite between glass layers");
        renderSpriteAt(6.0);
        check(frame.getPixel(32, 32) == core::ColorRGBA(120, 60, 60).pack(), "sprite behind both glass layers");
        renderSpriteAt(7.5);
        check(frame.getDepth(32, 32) == 5.5f, "opaque wall still occludes far sprite behind glass");

        // One opaque texel and one hole: column depth cannot describe this coverage.
        textures.set(2, {2, 1, {red, 0}});
        textures.set(3, {1, 1, {0}});
        renderSpriteAt(4.5);
        check(frame.getPixel(30, 32) == opaqueBlue && frame.getPixel(34, 32) == red,
              "sprite visible through cutout wall hole but hidden by opaque texel");
    }
}

int main() {
    try {
        textureAndPng();
        fogAndDepth();
        spritesAndOcclusion();
        wallIntegration();
        transparentLayers();
        transparentWallIntegration();
        std::cout << "LR2 rendering checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "LR2 rendering check failed: " << error.what() << '\n';
        return 1;
    }
}

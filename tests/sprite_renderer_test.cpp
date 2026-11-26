#include <SDL3/SDL.h>

#include <cstdio>
#include <limits>
#include <memory>
#include <stdexcept>

#include "wolf/render/Fog.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Player.hpp"

using namespace wolf;

namespace {

void check(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

void loadStrip(TextureStore& store, int id, Color left, Color right) {
    Color pixels[]{left, right};
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
        SDL_CreateSurfaceFrom(2, 1, SDL_PIXELFORMAT_RGBA32, pixels, sizeof(pixels)),
        SDL_DestroySurface);
    const char* path = "sprite_renderer_test_fixture.bmp";
    check(surface && SDL_SaveBMP(surface.get(), path), SDL_GetError());
    store.load(id, path);
    std::remove(path);
}

}

int main() {
    const Color red{200, 0, 0}, blue{0, 0, 200}, clear{0, 0, 0, 0};
    Texture texture{2, 2, {clear, red, blue, clear}};
    check(texture.sample(0, 1) == blue && texture.sample(1, 0) == red, "Texel order");
    check(texture.sample(-1, 0) == clear && texture.sample(0, 2) == clear, "Texel bounds");

    Fog fog;
    const Color source{200, 0, 100, 17};
    check(fog.apply(source, 2) == source, "Near fog");
    check(fog.apply(source, 10) == Color{150, 50, 100, 17}, "Fog interpolation");
    check(fog.apply(source, 20) == Color{100, 100, 100, 17}, "Far fog");

    TextureStore store;
    loadStrip(store, 101, red, clear);
    loadStrip(store, 102, blue, blue);
    FrameBuffer frame(12, 8);
    Player player;
    player.position = {0, 0};
    player.direction = {0, 1};
    player.cameraPlane = {-0.5, 0};
    const Color background{12, 23, 34};
    frame.clear(background);
    std::vector<double> walls(12, std::numeric_limits<double>::infinity());
    walls[3] = 2;
    // Both sprites cover the same 8x4 rectangle. Supply the near one first.
    const std::vector<Sprite> sprites{
        {{0, 2}, 101, 1}, {{0, 4}, 102, 2}, {{0, -1}, 103, 1},
        {{0, 0.0001}, 103, 1}
    };
    SpriteRenderer renderer;
    renderer.render(frame, sprites, player, store, walls, fog);
    for (int y = 0; y < frame.height(); ++y) {
        for (int x = 0; x < frame.width(); ++x) {
            Color expected = background;
            if (y >= 2 && y < 6 && x >= 2 && x < 10 && x != 3) {
                expected = x < 6 ? red : blue;
            }
            check(frame.data()[y * frame.width() + x] == expected,
                  "Projection, depth sorting, transparency or wall occlusion");
        }
    }

    frame.clear(background);
    // Center at the left edge. Only the right texel of the strip is visible.
    Fog dense{0, 4, {100, 100, 100}};
    renderer.render(frame, {{{1, 2}, 102, 1}}, player, store, walls, dense);
    check(frame.data()[4 * 12] == Color{50, 50, 150}, "Clipped sprite fog");
    check(frame.data()[4 * 12 + 4] == background, "Clipped sprite width");
}

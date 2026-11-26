#include <SDL3/SDL.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/Fog.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << "\n";
        std::exit(1);
    }
}

void loadQuadTexture(
    wolf::TextureStore& store,
    int id,
    wolf::Color topLeft,
    wolf::Color topRight,
    wolf::Color bottomLeft,
    wolf::Color bottomRight
) {
    wolf::Color pixels[]{topLeft, topRight, bottomLeft, bottomRight};
    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
        SDL_CreateSurfaceFrom(
            2,
            2,
            SDL_PIXELFORMAT_RGBA32,
            pixels,
            static_cast<int>(2 * sizeof(wolf::Color))
        ),
        SDL_DestroySurface
    );
    const char* path = "raycaster_test_fixture.bmp";
    check(surface && SDL_SaveBMP(surface.get(), path), SDL_GetError());
    store.load(id, path);
    std::remove(path);
}

} // namespace

int main() {
    wolf::Map map({
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    });

    wolf::Raycaster raycaster;

    const wolf::RayHit direct = raycaster.castRay(map, {1.5, 1.5}, {1.0, 0.0});
    check(direct.hit, "Direct ray missed wall");
    check(direct.mapX == 4 && direct.mapY == 1, "Direct ray wrong cell");
    check(direct.wallId == 1, "Direct ray wrong wallId");
    check(direct.side == wolf::WallSide::Vertical, "Direct ray wrong wall side");
    check(std::abs(direct.depth - 2.5) < 1e-9, "Direct ray wrong depth");
    check(std::abs(direct.wallU - 0.5) < 1e-9, "Direct ray wrong wallU");

    wolf::Player player;
    const double cameraX = 0.5;
    const wolf::Vec2 angledDir{
        player.direction.x + player.cameraPlane.x * cameraX,
        player.direction.y + player.cameraPlane.y * cameraX
    };
    const wolf::RayHit angled = raycaster.castRay(map, player.position, angledDir);
    check(angled.hit && angled.mapX == 4, "Angled ray did not hit front wall");
    check(std::abs(angled.depth - 2.5) < 1e-9, "Perpendicular depth mismatch (fisheye)");
    check(angled.wallU >= 0.0 && angled.wallU < 1.0, "wallU out of [0, 1)");

    const wolf::Vec2 relative{
        angled.hitPoint.x - player.position.x,
        angled.hitPoint.y - player.position.y
    };
    const double spriteConventionDepth =
        relative.x * player.direction.x + relative.y * player.direction.y;
    check(
        std::abs(angled.depth - spriteConventionDepth) < 1e-9,
        "Ray depth disagrees with sprite depth convention"
    );

    wolf::Map openMap({
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    });
    wolf::FrameBuffer openFrame(32, 24);
    raycaster.render(openFrame, openMap, player);
    const auto& openDepth = raycaster.getDepthBuffer();
    check(static_cast<int>(openDepth.size()) == openFrame.width(), "Depth buffer size mismatch");
    for (double d : openDepth) {
        check(std::isinf(d) && d > 0.0, "Open column depth must be +infinity");
    }

    // Проверка текстурирования стен, отражения textureX, затенения сторон и тумана (ЛР-2)
    wolf::TextureStore store;
    const wolf::Color red{200, 0, 0, 255};
    const wolf::Color green{0, 200, 0, 255};
    const wolf::Color blue{0, 0, 200, 255};
    const wolf::Color white{200, 200, 200, 255};
    const wolf::Color background{10, 20, 30, 255};
    loadQuadTexture(store, 1, red, green, blue, white);

    wolf::FrameBuffer frame(20, 20);
    frame.clear(background);
    const wolf::Fog noFog{10.0, 20.0, {100, 100, 100, 255}};
    raycaster.render(frame, map, player, store, noFog);

    auto pixelAt = [&](int x, int y) {
        return frame.data()[static_cast<std::size_t>(y) * frame.width() + x];
    };

    // Центральный столбец x = 10: стена x = 4, depth = 2.5, lineHeight = 8 -> y in [6, 14)
    check(pixelAt(10, 5) == background, "Ceiling pixel overwritten");
    check(pixelAt(10, 6) == green, "Top texel mismatch on vertical wall");
    check(pixelAt(10, 13) == white, "Bottom texel mismatch on vertical wall");
    check(pixelAt(10, 14) == background, "Floor pixel overwritten");
    check(std::abs(raycaster.getDepthBuffer()[10] - 2.5) < 1e-9, "Depth buffer value mismatch");

    // Горизонтальная стена (взгляд вдоль +Y): отражение textureX -> столбец 0, затенение 0.75 и туман 0.5
    wolf::Player southPlayer;
    southPlayer.position = {2.5, 1.5};
    southPlayer.direction = {0.0, 1.0};
    southPlayer.cameraPlane = {-0.5773502691896257, 0.0};

    frame.clear(background);
    const wolf::Fog halfFog{0.0, 5.0, {100, 100, 100, 255}};
    raycaster.render(frame, map, southPlayer, store, halfFog);

    // red(200,0,0) * 0.75 = (150,0,0); lerp с fog(100,100,100) при factor = 0.5 -> (125,50,50)
    check(
        pixelAt(10, 6) == wolf::Color{125, 50, 50, 255},
        "Horizontal wall UV flip, shading or fog mismatch"
    );

    std::cout << "Raycaster tests passed\n";
    return 0;
}

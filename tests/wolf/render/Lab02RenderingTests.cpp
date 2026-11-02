#include "wolf/effects/Fog.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SceneRenderer.hpp"
#include "wolf/render/TexturedWallRenderer.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>

namespace {
using namespace wolf;

void check(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}
template <typename F> void expectInvalid(F action) {
    try { action(); } catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("Expected invalid_argument");
}
TextureLookup lookup(const std::map<int, Texture>& textures) {
    return [&textures](int id) -> const Texture& { return textures.at(id); };
}
Player camera() {
    Player player;
    player.position = {0.0, 0.0};
    return player;
}
void testFog() {
    constexpr Color base = makeColor(200, 80, 10, 128);
    const FogSettings fog{2.0, 6.0, makeColor(0, 120, 220)};
    check(applyFog(base, 1.0, fog) == base, "No fog before start");
    check(applyFog(base, 2.0, fog) == base, "No fog at start");
    check(applyFog(base, 4.0, fog) == makeColor(100, 100, 115, 128), "Half fog must interpolate RGB");
    check(applyFog(base, 6.0, fog) == makeColor(0, 120, 220, 128), "Full fog at end");
    check(applyFog(base, std::numeric_limits<double>::infinity(), fog) ==
          makeColor(0, 120, 220, 128), "Distant fog");
    check(applyFog(base, std::numeric_limits<double>::quiet_NaN(), fog) == base,
          "Invalid depth must not change color");
    check(applyFog(base, 5, {5, 5, makeColor(0, 0, 0)}) == base,
          "Invalid fog range must not divide by zero");
}
void testWalls() {
    FrameBuffer frame(4, 8);
    const std::map<int, Texture> textures{{1, {2, 2, {
        makeColor(255, 0, 0), makeColor(0, 255, 0),
        makeColor(0, 0, 255), makeColor(255, 255, 0)}}}};
    std::vector<WallSlice> columns(4);
    columns[0].hit = {true, 1, 0, 0, 1, WallSide::West, 0.75};
    columns[0].height = 4;
    columns[0].drawStart = 2;
    columns[0].drawEnd = 6;
    columns[1] = columns[0];
    columns[1].hit.side = WallSide::North;
    columns[2] = columns[0];
    columns[2].hit.wallU = 0.0;
    columns[2].drawStart = -200;
    columns[2].drawEnd = 200;
    columns[2].height = 500;
    TexturedWallRenderer::drawFrame(frame, columns, lookup(textures), {20, 30, makeColor(0, 0, 0)});
    check(frame.pixelAt(0, 0) == makeColor(58, 65, 82), "Ceiling must be drawn");
    check(frame.pixelAt(0, 7) == makeColor(63, 56, 52), "Floor must be drawn");
    check(frame.pixelAt(0, 2) == makeColor(0, 255, 0), "U samples correct column");
    check(frame.pixelAt(0, 5) == makeColor(255, 255, 0), "V samples correct row");
    check(frame.pixelAt(1, 2) == makeColor(0, 191, 0), "Horizontal face darkening");
    check(frame.pixelAt(2, 0) != makeColor(58, 65, 82), "Large walls clip to top");
    check(frame.pixelAt(2, 7) != makeColor(63, 56, 52), "Large walls clip to bottom");
    check(frame.pixelAt(3, 2) == makeColor(58, 65, 82), "No wall preserves background");
    TexturedWallRenderer::drawFrame(frame, columns, lookup(textures),
                                    {0, 0.5, makeColor(5, 15, 25)});
    check(frame.pixelAt(0, 2) == makeColor(5, 15, 25), "Walls use camera depth for fog");
    expectInvalid([&] { TexturedWallRenderer::drawFrame(frame,
        std::span<const WallSlice>(columns.data(), 1), lookup(textures)); });
}
void testSpritesOcclusionAndOrdering() {
    FrameBuffer frame(12, 12);
    frame.clear(makeColor(10, 10, 10));
    const std::map<int, Texture> textures{
        {101, {1, 1, {makeColor(255, 0, 0)}}},
        {102, {1, 1, {makeColor(0, 0, 255)}}},
        {103, {1, 1, {makeColor(0, 255, 0, 0)}}},
        {104, {1, 1, {makeColor(0, 255, 0, 128)}}}
    };
    std::vector<double> depth(12, std::numeric_limits<double>::infinity());
    depth[5] = depth[6] = 1.0; // wall is closer than either billboard
    const std::vector<Sprite> objects{{{2, 0}, 101, 1}, {{4, 0}, 102, 2}};
    SpriteRenderer::drawSprites(frame, camera(), objects, depth, lookup(textures),
                                 {100, 200, makeColor(0, 0, 0)});
    check(frame.pixelAt(4, 6) == makeColor(255, 0, 0), "Near sprite draws over far sprite");
    check(frame.pixelAt(5, 6) == makeColor(10, 10, 10), "Wall must occlude sprite column");
    check(frame.pixelAt(6, 6) == makeColor(10, 10, 10), "Adjacent wall column must occlude sprite");
    check(frame.pixelAt(7, 6) == makeColor(255, 0, 0), "Sprite visible beside wall");
    check(frame.pixelAt(0, 6) == makeColor(10, 10, 10), "Outside sprite unaffected");
    expectInvalid([&] { SpriteRenderer::drawSprites(frame, camera(), objects,
        std::span<const double>(depth.data(), 2), lookup(textures)); });

    frame.clear(makeColor(10, 20, 30));
    depth.assign(12, std::numeric_limits<double>::infinity());
    const std::vector<Sprite> transparent{{{2, 0}, 103, 1}};
    SpriteRenderer::drawSprites(frame, camera(), transparent, depth, lookup(textures));
    check(frame.pixelAt(6, 6) == makeColor(10, 20, 30), "Alpha zero skips write");
    const std::vector<Sprite> halfAlpha{{{2, 0}, 104, 1}};
    SpriteRenderer::drawSprites(frame, camera(), halfAlpha, depth, lookup(textures),
                                 {100, 200, makeColor(0, 0, 0)});
    check(frame.pixelAt(6, 6) == makeColor(5, 138, 15), "Half alpha blends correctly");
    frame.clear(makeColor(10, 10, 10));
    const std::vector<Sprite> fogged{{{2, 0}, 101, 1}};
    SpriteRenderer::drawSprites(frame, camera(), fogged, depth, lookup(textures),
                                 {1, 2, makeColor(50, 60, 70)});
    check(frame.pixelAt(6, 6) == makeColor(50, 60, 70), "Distant sprite uses fog");
}
void testEdgeCases() {
    FrameBuffer frame(12, 12);
    frame.clear(makeColor(1, 2, 3));
    const std::map<int, Texture> textures{{101, {1, 1, {makeColor(255, 255, 255)}}}};
    const std::vector<double> depth(12, std::numeric_limits<double>::infinity());
    const std::vector<Sprite> sprites{
        {{-2, 0}, 101, 1}, // behind camera
        {{2, 0}, 101, 0}, // invalid scale
        {{2, 0}, 101, -3},
        {{2, 0}, 101, std::numeric_limits<double>::quiet_NaN()},
        {{2, 100}, 101, 1} // outside FOV
    };
    SpriteRenderer::drawSprites(frame, camera(), sprites, depth, lookup(textures));
    for (int y = 0; y < frame.height(); ++y) for (int x = 0; x < frame.width(); ++x) {
        check(frame.pixelAt(x, y) == makeColor(1, 2, 3), "Offscreen/invalid sprites must be ignored");
    }
    const std::vector<Sprite> clipped{{{0.05, 0}, 101, 1}};
    SpriteRenderer::drawSprites(frame, camera(), clipped, depth, lookup(textures));
    check(frame.pixelAt(0, 0) == makeColor(255, 255, 255), "Near billboard clips safely");
}
} // namespace

int main() {
    try {
        testFog();
        testWalls();
        testSpritesOcclusionAndOrdering();
        testEdgeCases();
        std::cout << "Lab 2 rendering tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/Fog.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SpriteRenderer.hpp"
#include "wolf/render/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

wolf::Map boxedMap()
{
    return {5, 5, {
        1, 1, 1, 1, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 1, 1, 1, 1
    }};
}

void testFrameBuffer()
{
    wolf::FrameBuffer frame(3, 3);
    frame.fillHorizon(0xFF112233, 0xFF445566);
    require(frame.data()[0] == 0xFF112233, "Ceiling color is missing");
    require(frame.data()[3] == 0xFF445566, "Floor color is missing");

    frame.setPixel(-1, 0, 0);
    frame.setPixel(3, 0, 0);
    frame.drawVerticalSpan(-1, 0, 2, 0);
    require(frame.data()[0] == 0xFF112233, "An out-of-bounds write changed the frame");

    frame.drawVerticalSpan(1, -10, 1, 0xFFFFFFFF);
    require(frame.data()[1] == 0xFFFFFFFF && frame.data()[4] == 0xFFFFFFFF,
            "Vertical span was not clipped to the frame");
    require(frame.data()[7] == 0xFF445566, "Clipped span changed the floor");

    frame.clear(0xFFABCDEF);
    for (int index = 0; index < 9; ++index)
    {
        require(frame.data()[index] == 0xFFABCDEF, "Clear did not fill every pixel");
    }
}

void testDdaAndPerspective()
{
    const wolf::Map map = boxedMap();
    const wolf::Raycaster raycaster;
    wolf::Player player;

    for (const double cameraX : {-1.0, 0.0, 1.0})
    {
        const wolf::RayHit hit = raycaster.cast(map, player, cameraX);
        require(hit.hit && hit.cellX == 0 && hit.wallId == 1 && hit.side == wolf::HitSide::X,
                "DDA missed the front wall");
        require(std::abs(hit.depth - 1.5) < 1e-9,
                "A flat wall must have the same perpendicular depth across the view");
    }

    player.direction = {0.0, 1.0};
    player.cameraPlane = {-0.66, 0.0};
    const wolf::RayHit yHit = raycaster.cast(map, player, 0.0);
    require(yHit.hit && yHit.cellY == 4 && yHit.side == wolf::HitSide::Y,
            "DDA did not report a horizontal wall side");
    require(std::abs(yHit.depth - 1.5) < 1e-9, "Y-wall depth is wrong");

    player.direction = {-2.0, 0.0};
    player.cameraPlane = {0.0, 1.32};
    const wolf::RayHit scaledHit = raycaster.cast(map, player, 1.0);
    require(std::abs(scaledHit.depth - 1.5) < 1e-9,
            "Depth must remain a world distance when direction is not unit length");

    std::vector<int> cells{
        1, 1, 1, 1, 1,
        1, 0, 0, 0, 1,
        1, 3, 0, 0, 1,
        1, 0, 0, 0, 1,
        1, 1, 1, 1, 1
    };
    const wolf::Map withInnerWall(5, 5, std::move(cells));
    player = {};
    const wolf::RayHit nearHit = raycaster.cast(withInnerWall, player, 0.0);
    require(nearHit.hit && nearHit.cellX == 1 && nearHit.cellY == 2 && nearHit.wallId == 3,
            "DDA did not stop at the first wall");
    require(std::abs(nearHit.depth - 0.5) < 1e-9, "First-wall depth is wrong");
    require(std::abs(nearHit.wallU - 0.5) < 1e-9, "Wrong texture coordinate on X wall");
    player.direction = {0.0, 1.0};
    player.cameraPlane = {-0.66, 0.0};
    player.position.x = 2.25;
    require(std::abs(raycaster.cast(map, player, 0.0).wallU - 0.25) < 1e-9,
            "Y wall should use the X coordinate for its texture");
}

void testProjectionAndClipping()
{
    const wolf::Map map = boxedMap();
    wolf::FrameBuffer frame(4, 10);
    wolf::Raycaster{}.render(map, wolf::Player{}, frame);

    const auto pixel = [&frame](int x, int y)
    {
        return frame.data()[y * frame.width() + x];
    };
    const auto ceiling = pixel(2, 0);
    const auto wall = pixel(2, 1);
    const auto floor = pixel(2, 9);
    require(ceiling != wall && floor != wall && ceiling != floor,
            "Ceiling, wall and floor must be distinguishable");
    require(pixel(2, 7) == wall && pixel(2, 8) == floor,
            "Wall height is not centered using perpendicular depth");
    for (int x = 0; x < frame.width(); ++x)
    {
        require(pixel(x, 1) == wall && pixel(x, 7) == wall,
                "The front wall height changes across the view");
    }
}

void testTexturedScene()
{
    wolf::TextureStore textures;
    textures.loadBmp(1, "assets/wolf/textures/brick.bmp");
    textures.loadBmp(4, "assets/wolf/textures/barrel.bmp", true);
    textures.loadBmp(5, "assets/wolf/textures/lamp.bmp", true);
    const wolf::Fog fog{2.0, 8.0};
    const wolf::Player player;
    const wolf::Raycaster raycaster;
    const wolf::SpriteRenderer sprites;
    wolf::FrameBuffer frame(80, 50);
    std::vector<double> depths(frame.width());

    raycaster.renderTextured(boxedMap(), player, textures, fog, frame, depths);
    require(std::abs(depths[40] - 1.5) < 1e-9, "Textured pass did not save wall depth");
    const auto& brick = textures.at(1);
    require(frame.data()[25 * 80 + 40] == brick.sample(brick.width / 2, brick.height / 2),
            "Textured wall did not sample the center of the image");
    require(fog.apply(0xFF000000, 2.0) == 0xFF000000
            && fog.apply(0xFF000000, 8.0) == fog.color
            && fog.apply(0xFF000000, 5.0) == 0xFF1F242C, "Fog gradient is wrong");

    const wolf::Sprite barrel{{1.5, 2.5}, 4, 0.8};
    const wolf::Sprite lamp{{1.2, 2.5}, 5, 0.8};
    const auto center = [&frame] { return frame.data()[25 * 80 + 40]; };
    const auto background = center();
    const auto transparentCorner = frame.data()[5 * 80 + 20];
    sprites.render(std::span(&barrel, 1), player, textures, depths, fog, frame);
    const auto barrelPixel = center();
    require(barrelPixel != background, "Front sprite is missing");
    require(frame.data()[5 * 80 + 20] == transparentCorner, "Transparent sprite background covered the wall");
    const wolf::Sprite nearFirst[]{barrel, lamp};
    sprites.render(nearFirst, player, textures, depths, fog, frame);
    require(center() == barrelPixel, "The farther sprite covered the nearer sprite");

    raycaster.renderTextured(boxedMap(), player, textures, fog, frame, depths);
    const auto rightBackground = frame.data()[25 * 80 + 42];
    const auto leftBackground = frame.data()[25 * 80 + 38];
    for (int x = 40; x < frame.width(); ++x)
    {
        depths[x] = 0.5;
    }
    sprites.render(std::span(&barrel, 1), player, textures, depths, fog, frame);
    require(frame.data()[25 * 80 + 42] == rightBackground, "Wall did not occlude the right half");
    require(frame.data()[25 * 80 + 38] != leftBackground, "Wall occluded the visible half");
}

} // namespace

int main()
{
    testFrameBuffer();
    testDdaAndPerspective();
    testProjectionAndClipping();
    testTexturedScene();
}

#include <cmath>
#include <cstdlib>
#include <iostream>

#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << "\n";
        std::exit(1);
    }
}

}

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
    wolf::FrameBuffer frame(32, 24);
    raycaster.render(frame, openMap, player);
    const auto& depthBuffer = raycaster.getDepthBuffer();
    check(static_cast<int>(depthBuffer.size()) == frame.width(), "Depth buffer size mismatch");
    for (double d : depthBuffer) {
        check(std::isinf(d) && d > 0.0, "Open column depth must be +infinity");
    }

    std::cout << "Raycaster tests passed\n";
    return 0;
}

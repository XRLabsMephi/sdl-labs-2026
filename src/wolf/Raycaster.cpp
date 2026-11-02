#include "wolf/raycast/Raycaster.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"
#include <cmath>
#include <limits>
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/core/Color.hpp"

Vec2 Raycaster::rayDirection(const Player& player, double cameraX) const {
    return player.direction + player.cameraPlane * cameraX;
}

RayHit Raycaster::castRay(const Map& map, const Player& player, double cameraX) const {
    Vec2 direction = rayDirection(player, cameraX);
    int mapX = static_cast<int>(std::floor(player.position.x));
    int mapY = static_cast<int>(std::floor(player.position.y));
    double infinity = std::numeric_limits<double>::infinity();
    double deltaDistX;
    double deltaDistY;
    if (direction.x == 0.0) deltaDistX = infinity;
    else deltaDistX = std::abs(1.0 / direction.x);
    if (direction.y == 0.0) deltaDistY = infinity;
    else deltaDistY = std::abs(1.0 / direction.y);
    int stepX;
    int stepY;
    double sideDistX;
    double sideDistY;
    if (direction.x < 0.0) {
        stepX = -1;
        sideDistX = (player.position.x - mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - player.position.x) * deltaDistX;
    }
    if (direction.y < 0.0) {
        stepY = -1;
        sideDistY = (player.position.y - mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - player.position.y) * deltaDistY;
    }
    RayHit hit;
    while (true) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            hit.side = HitSide::X;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            hit.side = HitSide::Y;
        }
        if (map.isWall(mapX, mapY)) {
            hit.hit = true;
            hit.mapX = mapX;
            hit.mapY = mapY;
            hit.wallId = map.getCell(mapX, mapY);
            if (hit.side == HitSide::X) hit.depth = sideDistX - deltaDistX;
            else hit.depth = sideDistY - deltaDistY;
            return hit;
        }
    }
}

void Raycaster::render(const Map& map, const Player& player, FrameBuffer& frame) const {
    Color ceiling = makeColor(60, 60, 60);
    Color floor = makeColor(100, 100, 100);
    frame.clear(ceiling);
    for (int y = frame.height() / 2; y < frame.height(); y++) {
        for (int x = 0; x < frame.width(); x++) {
            frame.setPixel(x, y, floor);
        }
    }
    for (int x = 0; x < frame.width(); x++) {
        double cameraX = 2.0 * x / static_cast<double>(frame.width()) - 1.0;
        RayHit hit = castRay(map, player, cameraX);
        if (!hit.hit || hit.depth <= 0.0) continue;
        int wallHeight = static_cast<int>(frame.height() / hit.depth);
        int start = frame.height() / 2 - wallHeight / 2;
        int finish = frame.height() / 2 + wallHeight / 2;
        if (start < 0) start = 0;
        if (finish >= frame.height()) finish = frame.height() - 1;
        Color color;
        if (hit.wallId == 1) color = makeColor(200, 50, 50);
        else if (hit.wallId == 2) color = makeColor(50, 200, 50);
        else if (hit.wallId == 3) color = makeColor(50, 50, 200);
        else if (hit.wallId == 4) color = makeColor(200, 200, 50);
        else color = makeColor(200, 200, 200);
        // if (hit.side == HitSide::Y) {
        //     color = makeColor(
        //         static_cast<std::uint8_t>((color >> 16 & 255) / 2),
        //         static_cast<std::uint8_t>((color >> 8 & 255) / 2),
        //         static_cast<std::uint8_t>((color & 255) / 2)
        //     );
        // }
        for (int y = start; y <= finish; y++) {
            frame.setPixel(x, y, color);
        }
    }
}
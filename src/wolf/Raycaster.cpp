#include "wolf/raycast/Raycaster.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include <cmath>
#include <limits>

Vec2 Raycaster::rayDirection(const Player &player, double cameraX) const {
    return player.direction + player.cameraPlane * cameraX;
}

RayHit Raycaster::castRay(const Map& map, const Player& player, double cameraX) const {
    Vec2 direction = rayDirection(player, cameraX);
    int mapX = static_cast<int>(std::floor(player.position.x));
    int mapY = static_cast<int>(std::floor(player.position.y));
    if (!map.isInside(mapX, mapY)) return {};
    if (map.isWall(mapX, mapY)) return {};
    if (direction.x == 0.0 && direction.y == 0.0) return {};
    double infinity = std::numeric_limits<double>::infinity();
    double deltaDistX;
    double deltaDistY;
    if (direction.x == 0.0) deltaDistX = infinity;
    else deltaDistX = std::abs(1.0 / direction.x);
    if (direction.y == 0.0) deltaDistY = infinity;
    else deltaDistY = std::abs(1.0 / direction.y);
    int stepX;
    int stepY;
    if (direction.x < 0.0) stepX = -1;
    else stepX = 1;
    if (direction.y < 0.0) stepY = -1;
    else stepY = 1;
    double sideDistX;
    double sideDistY;
    if (direction.x < 0.0) sideDistX = (player.position.x - mapX) * deltaDistX;
    else sideDistX = (mapX + 1.0 - player.position.x) * deltaDistX;
    if (direction.y < 0.0) sideDistY = (player.position.y - mapY) * deltaDistY;
    else sideDistY = (mapY + 1.0 - player.position.y) * deltaDistY;
    RayHit result;
    while (true) {
        if (sideDistX < sideDistY) {
            result.depth = sideDistX;
            sideDistX += deltaDistX;
            mapX += stepX;
            result.side = HitSide::X;
        } else {
            result.depth = sideDistY;
            sideDistY += deltaDistY;
            mapY += stepY;
            result.side = HitSide::Y;
        }
        if (!map.isInside(mapX, mapY)) return {};
        if (map.isWall(mapX, mapY)) {
            result.hit = true;
            result.mapX = mapX;
            result.mapY = mapY;
            result.wallId = map.getCell(mapX, mapY);
            return result;
        }
    }
}

void Raycaster::render(const Map& map, const Player& player, FrameBuffer& frame) const {
    Color ceilingColor = makeColor(70, 100, 140);
    Color floorColor = makeColor(90, 90, 90);
    frame.clear(floorColor);
    for (int y = 0; y < frame.height() / 2; ++y) {
        for (int x = 0; x < frame.width(); ++x) {
            frame.setPixel(x, y, ceilingColor);
        }
    }
    for (int x = 0; x < frame.width(); ++x) {
        double cameraX = 2.0 * x / frame.width() - 1.0;
        RayHit hit = castRay(map, player, cameraX);
        if (!hit.hit) continue;
        double depth = hit.depth;
        if (depth < 0.0001) depth = 0.0001;
        double wallHeight = frame.height() / depth;
        double top = (frame.height() - wallHeight) / 2.0;
        double bottom = (frame.height() + wallHeight) / 2.0;
        if (top < 0.0) top = 0.0;
        if (bottom > frame.height()) bottom = frame.height();
        int drawStart = static_cast<int>(std::floor(top));
        int drawEnd = static_cast<int>(std::ceil(bottom));
        int red = 200;
        int green = 70;
        int blue = 70;
        if (hit.wallId == 2) {
            red = 70;
            green = 180;
            blue = 90;
        } else if (hit.wallId == 3) {
            red = 70;
            green = 100;
            blue = 200;
        } else if (hit.wallId == 4) {
            red = 200;
            green = 180;
            blue = 70;
        }
        if (hit.side == HitSide::Y) {
            red /= 2;
            green /= 2;
            blue /= 2;
        }
        Color wallColor = makeColor(red, green, blue);
        for (int y = drawStart; y < drawEnd; ++y) {
            frame.setPixel(x, y, wallColor);
        }
    }
}

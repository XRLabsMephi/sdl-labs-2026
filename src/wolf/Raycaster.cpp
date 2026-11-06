#include "wolf/raycast/Raycaster.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
    bool validTexture(const Texture* texture) {
        if (!texture || texture->width <= 0 || texture->height <= 0) return false;
        std::size_t width = static_cast<std::size_t>(texture->width);
        return texture->pixels.size() / width == static_cast<std::size_t>(texture->height) && texture->pixels.size() % width == 0;
    }
}

Vec2 Raycaster::rayDirection(const Player& player, double cameraX) const {
    return player.direction + player.cameraPlane * cameraX;
}

RayHit Raycaster::castRay(const Map& map, const Player& player, double cameraX) const {
    Vec2 direction = rayDirection(player, cameraX);

    if (!std::isfinite(player.position.x) || !std::isfinite(player.position.y)) return {};
    if (!std::isfinite(direction.x) || !std::isfinite(direction.y)) return {};
    if (player.position.x < 0.0 || player.position.y < 0.0 || player.position.x >= map.getWidth() || player.position.y >= map.getHeight()) return {};
    if (direction.x == 0.0 && direction.y == 0.0) return {};

    int mapX = static_cast<int>(std::floor(player.position.x));
    int mapY = static_cast<int>(std::floor(player.position.y));
    if (map.isWall(mapX, mapY)) return {};

    double infinity = std::numeric_limits<double>::infinity();
    double deltaDistX = infinity;
    double deltaDistY = infinity;

    if (direction.x != 0.0) deltaDistX = std::abs(1.0 / direction.x);
    if (direction.y != 0.0) deltaDistY = std::abs(1.0 / direction.y);

    int stepX = 1;
    int stepY = 1;
    double sideDistX = infinity;
    double sideDistY = infinity;

    if (direction.x < 0.0) {
        stepX = -1;
        sideDistX = (player.position.x - mapX) * deltaDistX;
    } else if (direction.x > 0.0) {
        sideDistX = (mapX + 1.0 - player.position.x) * deltaDistX;
    }

    if (direction.y < 0.0) {
        stepY = -1;
        sideDistY = (player.position.y - mapY) * deltaDistY;
    } else if (direction.y > 0.0) {
        sideDistY = (mapY + 1.0 - player.position.y) * deltaDistY;
    }

    RayHit hit;

    while (true) {
        if (sideDistX < sideDistY) {
            hit.depth = sideDistX;
            sideDistX += deltaDistX;
            mapX += stepX;
            hit.side = HitSide::X;
        } else {
            hit.depth = sideDistY;
            sideDistY += deltaDistY;
            mapY += stepY;
            hit.side = HitSide::Y;
        }

        if (!map.isInside(mapX, mapY)) return {};

        if (map.isWall(mapX, mapY)) {
            if (!std::isfinite(hit.depth)) return {};

            hit.hit = true;
            hit.mapX = mapX;
            hit.mapY = mapY;
            hit.wallId = map.getCell(mapX, mapY);

            double wallCoordinate;
            if (hit.side == HitSide::X) wallCoordinate = player.position.y + hit.depth * direction.y;
            else wallCoordinate = player.position.x + hit.depth * direction.x;

            hit.wallU = wallCoordinate - std::floor(wallCoordinate);
            return hit;
        }
    }
}

void Raycaster::render(const Map& map, const Player& player, FrameBuffer& frame, const TextureStore& textures, const Fog& fog) {
    Color ceilingColor = makeColor(70, 100, 140);
    Color floorColor = makeColor(90, 90, 90);

    frame.clear(floorColor);
    depthBuffer_.assign(frame.width(), std::numeric_limits<double>::infinity());

    for (int y = 0; y < frame.height() / 2; ++y) {
        for (int x = 0; x < frame.width(); ++x) {
            frame.setPixel(x, y, ceilingColor);
        }
    }

    for (int x = 0; x < frame.width(); ++x) {
        double cameraX = 2.0 * x / frame.width() - 1.0;
        RayHit hit = castRay(map, player, cameraX);
        if (!hit.hit) continue;

        depthBuffer_[x] = hit.depth;

        double depth = std::max(hit.depth, 0.0001);
        double wallHeight = frame.height() / depth;
        double top = (frame.height() - wallHeight) / 2.0;
        double bottom = (frame.height() + wallHeight) / 2.0;

        int drawStart = static_cast<int>(std::floor(std::max(0.0, top)));
        int drawEnd = static_cast<int>(std::ceil(std::min(static_cast<double>(frame.height()), bottom)));

        const Texture* texture = textures.get(hit.wallId);
        if (!validTexture(texture)) texture = nullptr;

        int textureX = 0;

        if (texture) {
            textureX = std::clamp(static_cast<int>(hit.wallU * texture->width), 0, texture->width - 1);

            Vec2 direction = rayDirection(player, cameraX);
            if ((hit.side == HitSide::X && direction.x > 0.0) || (hit.side == HitSide::Y && direction.y < 0.0)) textureX = texture->width - textureX - 1;
        }

        Color fallback = makeColor(200, 70, 70);
        if (hit.wallId == 2) fallback = makeColor(70, 180, 90);
        else if (hit.wallId == 3) fallback = makeColor(70, 100, 200);
        else if (hit.wallId == 4) fallback = makeColor(200, 180, 70);

        for (int y = drawStart; y < drawEnd; ++y) {
            Color color = fallback;

            if (texture) {
                double v = std::clamp((y + 0.5 - top) / wallHeight, 0.0, 1.0);
                int textureY = std::clamp(static_cast<int>(v * texture->height), 0, texture->height - 1);
                color = texture->pixels[static_cast<std::size_t>(textureY) * texture->width + textureX];
            }

            int red = (color >> 16) & 255;
            int green = (color >> 8) & 255;
            int blue = color & 255;

            if (hit.side == HitSide::Y) {
                red /= 2;
                green /= 2;
                blue /= 2;
            }

            Color wallColor = fog.apply(makeColor(red, green, blue), hit.depth);
            frame.setPixel(x, y, wallColor);
        }
    }
}

const std::vector<double>& Raycaster::depthBuffer() const {
    return depthBuffer_;
}
#include "wolf/raycast/Raycaster.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace wolf {

RayHit Raycaster::castRay(const Map& map, const Vec2& origin, const Vec2& dir) const {
    RayHit result;

    int mapX = static_cast<int>(std::floor(origin.x));
    int mapY = static_cast<int>(std::floor(origin.y));

    const double deltaDistX = (std::abs(dir.x) < 1e-9) ? 1e30 : std::abs(1.0 / dir.x);
    const double deltaDistY = (std::abs(dir.y) < 1e-9) ? 1e30 : std::abs(1.0 / dir.y);

    double sideDistX = 0.0;
    double sideDistY = 0.0;
    int stepX = 0;
    int stepY = 0;

    if (dir.x < 0.0) {
        stepX = -1;
        sideDistX = (origin.x - mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - origin.x) * deltaDistX;
    }

    if (dir.y < 0.0) {
        stepY = -1;
        sideDistY = (origin.y - mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - origin.y) * deltaDistY;
    }

    bool hit = false;
    WallSide hitSide = WallSide::Vertical;

    while (!hit) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            hitSide = WallSide::Vertical;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            hitSide = WallSide::Horizontal;
        }

        if (!map.in_bounds(mapX, mapY)) {
            break;
        }

        const int tile = map.wall_id(mapX, mapY);
        if (tile > 0) {
            hit = true;
            result.hit = true;
            result.mapX = mapX;
            result.mapY = mapY;
            result.wallId = tile;
            result.side = hitSide;
        }
    }

    if (result.hit) {
        if (hitSide == WallSide::Vertical) {
            result.depth = sideDistX - deltaDistX;
        } else {
            result.depth = sideDistY - deltaDistY;
        }

        result.hitPoint = Vec2{
            origin.x + dir.x * result.depth,
            origin.y + dir.y * result.depth
        };

        double wallX = (hitSide == WallSide::Vertical)
            ? (origin.y + result.depth * dir.y)
            : (origin.x + result.depth * dir.x);
        wallX -= std::floor(wallX);
        result.wallU = std::clamp(wallX, 0.0, std::nextafter(1.0, 0.0));
    } else {
        result.depth = std::numeric_limits<double>::infinity();
    }

    return result;
}

void Raycaster::render(FrameBuffer& fb, const Map& map, const Player& player) {
    const int screenWidth = fb.width();
    const int screenHeight = fb.height();

    m_depthBuffer.assign(
        static_cast<std::size_t>(screenWidth),
        std::numeric_limits<double>::infinity()
    );

    for (int x = 0; x < screenWidth; ++x) {
        const double cameraX = 2.0 * x / static_cast<double>(screenWidth) - 1.0;
        const Vec2 rayDir{
            player.direction.x + player.cameraPlane.x * cameraX,
            player.direction.y + player.cameraPlane.y * cameraX
        };

        const RayHit hit = castRay(map, player.position, rayDir);
        m_depthBuffer[static_cast<std::size_t>(x)] = hit.depth;
        if (!hit.hit || hit.depth <= 0.0001) {
            continue;
        }

        const int lineHeight = static_cast<int>(screenHeight / hit.depth);
        if (lineHeight <= 0) {
            continue;
        }

        const int drawStart = -lineHeight / 2 + screenHeight / 2;
        const int drawEnd = drawStart + lineHeight;
        const int clampedStart = std::max(0, drawStart);
        const int clampedEnd = std::min(screenHeight, drawEnd);

        const Color wallColor = getWallColor(hit.wallId, hit.side);
        for (int y = clampedStart; y < clampedEnd; ++y) {
            fb.setPixel(x, y, wallColor);
        }
    }
}

void Raycaster::render(
    FrameBuffer& frame,
    const Map& map,
    const Player& player,
    const TextureStore& textures,
    const Fog& fog
) {
    const int screenWidth = frame.width();
    const int screenHeight = frame.height();

    m_depthBuffer.assign(
        static_cast<std::size_t>(screenWidth),
        std::numeric_limits<double>::infinity()
    );

    for (int x = 0; x < screenWidth; ++x) {
        const double cameraX = 2.0 * x / static_cast<double>(screenWidth) - 1.0;
        const Vec2 rayDir{
            player.direction.x + player.cameraPlane.x * cameraX,
            player.direction.y + player.cameraPlane.y * cameraX
        };

        const RayHit hit = castRay(map, player.position, rayDir);
        m_depthBuffer[static_cast<std::size_t>(x)] = hit.depth;

        if (!hit.hit || hit.depth <= 0.0001) {
            continue;
        }

        const int lineHeight = static_cast<int>(screenHeight / hit.depth);
        if (lineHeight <= 0) {
            continue;
        }

        const Texture& texture = textures.get(hit.wallId);
        if (texture.width <= 0 || texture.height <= 0) {
            continue;
        }

        const int drawStart = -lineHeight / 2 + screenHeight / 2;
        const int drawEnd = drawStart + lineHeight;
        const int clampedStart = std::max(0, drawStart);
        const int clampedEnd = std::min(screenHeight, drawEnd);

        int textureX = static_cast<int>(hit.wallU * static_cast<double>(texture.width));
        if ((hit.side == WallSide::Vertical && rayDir.x < 0.0) ||
            (hit.side == WallSide::Horizontal && rayDir.y > 0.0)) {
            textureX = texture.width - textureX - 1;
        }
        textureX = std::clamp(textureX, 0, texture.width - 1);

        const double step = static_cast<double>(texture.height) / static_cast<double>(lineHeight);
        double texPos = static_cast<double>(clampedStart - drawStart) * step;

        for (int y = clampedStart; y < clampedEnd; ++y) {
            const int textureY = std::clamp(static_cast<int>(texPos), 0, texture.height - 1);
            texPos += step;

            Color color = texture.sample(textureX, textureY);
            if (hit.side == WallSide::Horizontal) {
                color.r = static_cast<std::uint8_t>(color.r * 0.75);
                color.g = static_cast<std::uint8_t>(color.g * 0.75);
                color.b = static_cast<std::uint8_t>(color.b * 0.75);
            }

            frame.setPixel(x, y, fog.apply(color, hit.depth));
        }
    }
}

const std::vector<double>& Raycaster::getDepthBuffer() const {
    return m_depthBuffer;
}

Color Raycaster::getWallColor(int wallId, WallSide side) const {
    Color base;
    switch (wallId) {
        case 1:  base = Color{200, 50, 50, 255};   break;
        case 2:  base = Color{50, 200, 50, 255};   break;
        case 3:  base = Color{50, 50, 200, 255};   break;
        case 4:  base = Color{200, 200, 200, 255}; break;
        default: base = Color{180, 180, 50, 255};  break;
    }

    if (side == WallSide::Horizontal) {
        base.r = static_cast<std::uint8_t>(base.r * 0.7);
        base.g = static_cast<std::uint8_t>(base.g * 0.7);
        base.b = static_cast<std::uint8_t>(base.b * 0.7);
    }
    return base;
}

} // namespace wolf

#include "wolf/raycast/Raycaster.hpp"

#include <algorithm>
#include <cmath>

namespace wolf
{
RayHit Raycaster::castRay(const Map& map, const Player& player, int screenX, int screenWidth) const
{
    RayHit hit;

    const double cameraX = 2.0 * ((screenX + 0.5) / static_cast<double>(screenWidth)) - 1.0;
    const Vec2 rayDir = player.direction + player.cameraPlane * cameraX;

    int mapX = static_cast<int>(std::floor(player.position.x)), mapY = static_cast<int>(std::floor(player.position.y));

    const double safeRayDirX = std::abs(rayDir.x) < 1e-9 ? (rayDir.x < 0.0 ? -1e-9 : 1e-9) : rayDir.x;
    const double safeRayDirY = std::abs(rayDir.y) < 1e-9 ? (rayDir.y < 0.0 ? -1e-9 : 1e-9) : rayDir.y;

    const double deltaDistX = std::abs(1.0 / safeRayDirX);
    const double deltaDistY = std::abs(1.0 / safeRayDirY);

    int stepX, stepY;
    double sideDistX, sideDistY;

    if (rayDir.x < 0.0)
    {
        stepX = -1;
        sideDistX = (player.position.x - mapX) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapX + 1.0 - player.position.x) * deltaDistX;
    }

    if (rayDir.y < 0.0)
    {
        stepY = -1;
        sideDistY = (player.position.y - mapY) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapY + 1.0 - player.position.y) * deltaDistY;
    }

    int side = 0;

    for (int iteration = 0; iteration < 256; ++iteration)
    {
        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        }
        else
        {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }

        if (map.isWall(mapX, mapY))
        {
            hit.hit = true;
            break;
        }
    }

    if (!hit.hit)
    {
        hit.depth = 1000.0;
        return hit;
    }

    if (side == 0)
    {
        hit.depth = (mapX - player.position.x + (1 - stepX) / 2.0) / safeRayDirX;
    }
    else
    {
        hit.depth = (mapY - player.position.y + (1 - stepY) / 2.0) / safeRayDirY;
    }

    hit.depth = std::max(0.0001, hit.depth);
    hit.position = player.position + rayDir * hit.depth;
    hit.mapX = mapX;
    hit.mapY = mapY;
    hit.wallId = map.cell(mapX, mapY);
    hit.side = side;

    return hit;
}

Color Raycaster::wallColor(int wallId, int side) const
{
    Color color = makeColor(71, 85, 105);

    if (wallId == 2)
    {
        color = makeColor(194, 120, 64);
    }
    else if (wallId == 3)
    {
        color = makeColor(46, 125, 92);
    }

    if (side == 1)
    {
        color = shadeColor(color, 0.72);
    }

    return color;
}

void Raycaster::render(const Map& map, const Player& player, FrameBuffer& frameBuffer)
{
    const Color ceilingColor = makeColor(186, 230, 253);
    const Color floorColor = makeColor(148, 163, 184);

    const int screenWidth = frameBuffer.width(), screenHeight = frameBuffer.height();

    frameBuffer.clear(ceilingColor);

    for (int y = screenHeight / 2; y < screenHeight; ++y)
    {
        for (int x = 0; x < screenWidth; ++x)
        {
            frameBuffer.setPixel(x, y, floorColor);
        }
    }

    depthBuffer_.assign(screenWidth, 1000.0);

    const double planeLength = std::sqrt(player.cameraPlane.x * player.cameraPlane.x + player.cameraPlane.y * player.cameraPlane.y);
    const double projectionDistance = (screenWidth / 2.0) / planeLength;

    for (int x = 0; x < screenWidth; ++x)
    {
        const RayHit hit = castRay(map, player, x, screenWidth);

        depthBuffer_[x] = hit.depth;

        if (!hit.hit)
        {
            continue;
        }

        const int wallHeight = static_cast<int>(projectionDistance / hit.depth);
        const int drawStart = screenHeight / 2 - wallHeight / 2;
        const int drawEnd = screenHeight / 2 + wallHeight / 2;

        frameBuffer.fillColumn(x, drawStart, drawEnd, wallColor(hit.wallId, hit.side));
    }
}

const std::vector<double>& Raycaster::depthBuffer() const
{
    return depthBuffer_;
}
}

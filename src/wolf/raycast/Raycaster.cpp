#include "wolf/raycast/Raycaster.hpp"

#include "wolf/render/FrameBuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace wolf
{

namespace
{

constexpr FrameBuffer::Pixel ceilingColor = 0xFF3D4858;
constexpr FrameBuffer::Pixel floorColor = 0xFF585047;

FrameBuffer::Pixel wallColor(int wallId, HitSide side)
{
    constexpr std::array<FrameBuffer::Pixel, 6> colors{
        0xFFC76656, 0xFF6BAF77, 0xFF638CBF,
        0xFFD0A95E, 0xFFA37DB8, 0xFF70B5B5
    };
    const auto color = colors[(static_cast<std::uint32_t>(wallId) - 1U) % colors.size()];
    if (side == HitSide::X)
    {
        return color;
    }

    // Darken one wall orientation to make room corners easier to read.
    const auto shade = [](FrameBuffer::Pixel channel)
    {
        return channel * 7U / 10U;
    };
    return 0xFF000000U
        | (shade((color >> 16U) & 0xFFU) << 16U)
        | (shade((color >> 8U) & 0xFFU) << 8U)
        | shade(color & 0xFFU);
}

} // namespace

RayHit Raycaster::cast(const Map& map, const Player& player, double cameraX) const
{
    RayHit result;
    const auto& position = player.position;
    if (!std::isfinite(position.x) || !std::isfinite(position.y)
        || !std::isfinite(cameraX)
        || position.x < 0.0 || position.x >= map.width()
        || position.y < 0.0 || position.y >= map.height())
    {
        return result;
    }

    const auto forward = player.direction.normalized();
    if (!std::isfinite(forward.x) || !std::isfinite(forward.y)
        || (forward.x == 0.0 && forward.y == 0.0))
    {
        return result;
    }

    const auto rayDirection = player.direction + player.cameraPlane * cameraX;
    if (!std::isfinite(rayDirection.x) || !std::isfinite(rayDirection.y)
        || (rayDirection.x == 0.0 && rayDirection.y == 0.0))
    {
        return result;
    }

    int cellX = static_cast<int>(std::floor(position.x));
    int cellY = static_cast<int>(std::floor(position.y));
    if (map.isWall(cellX, cellY))
    {
        return {true, 0.0, cellX, cellY, map.tileAt(cellX, cellY), HitSide::X};
    }

    const double infinity = std::numeric_limits<double>::infinity();
    const double deltaX = rayDirection.x == 0.0 ? infinity : std::abs(1.0 / rayDirection.x);
    const double deltaY = rayDirection.y == 0.0 ? infinity : std::abs(1.0 / rayDirection.y);
    const int stepX = rayDirection.x < 0.0 ? -1 : 1;
    const int stepY = rayDirection.y < 0.0 ? -1 : 1;
    double sideX = (rayDirection.x < 0.0 ? position.x - cellX : cellX + 1.0 - position.x) * deltaX;
    double sideY = (rayDirection.y < 0.0 ? position.y - cellY : cellY + 1.0 - position.y) * deltaY;

    // Even without border walls, leaving the finite map takes at most this many cells.
    const auto maxSteps = static_cast<long long>(map.width()) + map.height() + 1;
    for (long long step = 0; step < maxSteps; ++step)
    {
        double distanceAlongRay;
        if (sideX <= sideY)
        {
            distanceAlongRay = sideX;
            sideX += deltaX;
            cellX += stepX;
            result.side = HitSide::X;
        }
        else
        {
            distanceAlongRay = sideY;
            sideY += deltaY;
            cellY += stepY;
            result.side = HitSide::Y;
        }

        const int wallId = map.tileAt(cellX, cellY);
        if (wallId != 0)
        {
            // Project the hit onto the forward direction, avoiding fish-eye distortion.
            const double forwardComponent = rayDirection.x * forward.x + rayDirection.y * forward.y;
            result.hit = true;
            result.depth = distanceAlongRay * forwardComponent;
            result.cellX = cellX;
            result.cellY = cellY;
            result.wallId = wallId;
            return result;
        }
    }
    return {};
}

void Raycaster::render(const Map& map, const Player& player, FrameBuffer& frameBuffer) const
{
    frameBuffer.fillHorizon(ceilingColor, floorColor);
    const int screenHeight = frameBuffer.height();
    for (int x = 0; x < frameBuffer.width(); ++x)
    {
        const double cameraX = 2.0 * x / frameBuffer.width() - 1.0;
        const RayHit hit = cast(map, player, cameraX);
        if (!hit.hit || !std::isfinite(hit.depth) || hit.depth < 0.0)
        {
            continue;
        }

        const double projectedHeight = screenHeight / std::max(hit.depth, 1e-6);
        const auto color = wallColor(hit.wallId, hit.side);
        if (projectedHeight >= screenHeight)
        {
            frameBuffer.drawVerticalSpan(x, 0, screenHeight - 1, color);
            continue;
        }

        const int stripeHeight = std::max(1, static_cast<int>(std::round(projectedHeight)));
        const int top = (screenHeight - stripeHeight) / 2;
        frameBuffer.drawVerticalSpan(x, top, top + stripeHeight - 1, color);
    }
}

} // namespace wolf

#include "wolf/raycast/Raycaster.hpp"

#include <cmath>
#include <cstdint>

#include "wolf/core/color.hpp"

namespace {
    constexpr double kSideShadeFactor = 0.7;   // shade y-sides
    constexpr double kEpsilon  = 1e-9;         // 0 division
    constexpr double kHugeDist = 1e30;         // no hit
    constexpr int    kMaxSteps = 1024;         // DDA limit

    // Colors
    core::ColorRGBA wallColor(int wallId, int side) {
        core::ColorRGBA color;

        switch (wallId) {
            case 1:  color = {110, 120, 145}; break;
            case 2:  color = {190,  95,  75}; break;
            case 3:  color = { 90, 160, 100}; break;
            default: color = {120, 120, 120}; break;
        }

        if (side == 1) {
            color.r = static_cast<std::uint8_t>(color.r * kSideShadeFactor);
            color.g = static_cast<std::uint8_t>(color.g * kSideShadeFactor);
            color.b = static_cast<std::uint8_t>(color.b * kSideShadeFactor);
        }

        return color;
    }
}

namespace raycast {
    RayHit Raycaster::castRay(const world::Player& player,
                              const world::Map&    map,
                              double               cameraX) const {
        RayHit result{};

        const core::Vec2 pos   = player.position();
        const core::Vec2 dir   = player.direction();
        const core::Vec2 plane = player.cameraPlane();

        // Resulting ray
        const core::Vec2 rayDir = dir + plane * cameraX;

        int mapX = static_cast<int>(std::floor(pos.x));
        int mapY = static_cast<int>(std::floor(pos.y));

        // Player is inside wall or outside of the map
        if (map.isSolid(mapX, mapY)) {
            result.hit    = true;
            result.depth  = kEpsilon;
            result.mapX   = mapX;
            result.mapY   = mapY;
            result.wallId = map.cellAt(mapX, mapY);
            return result;
        }

        // where to move to get the first
        int sgnX = (rayDir.x > kEpsilon) ? 1 : ((rayDir.x < -kEpsilon) ? -1 : 0),
            sgnY = (rayDir.y > kEpsilon) ? 1 : ((rayDir.y < -kEpsilon) ? -1 : 0);


        if (sgnX == 0 && sgnY == 0) {
            result.depth = kHugeDist;
            return result;
        }

        // slope
        double k    = (sgnX != 0) ? rayDir.y / rayDir.x : 0.0;
        double invK = (sgnY != 0) ? rayDir.x / rayDir.y : 0.0;

        core::Vec2 LastPoint = pos;

        for (int i = 0; i < kMaxSteps; ++i) {
            double FirstX = mapX + ((rayDir.x > 0) ? 1.0 : 0.0); // first intersection with x
            double FirstY = mapY + ((rayDir.y > 0) ? 1.0 : 0.0); // first intersection with y

            core::Vec2 PointX{FirstX, LastPoint.y + (FirstX - LastPoint.x) * k},    // on x intersection
                       PointY{LastPoint.x + (FirstY - LastPoint.y) * invK, FirstY}; // on y intersection


            if (sgnY == 0) {
                // horizontal ray: only x intersections happen
                LastPoint = PointX;
                mapX += sgnX;
                result.side = 0;
            }
            else if (sgnX == 0) {
                // vertical ray: only y intersections happen
                LastPoint = PointY;
                mapY += sgnY;
                result.side = 1;
            }
            else if (sgnX == 1) {
                if (PointX.x < PointY.x) {
                    LastPoint = PointX;
                    mapX += sgnX;
                    result.side = 0;
                }
                else {
                    LastPoint = PointY;
                    mapY += sgnY;
                    result.side = 1;
                }
            }
            else if (sgnX == -1) {
                if (PointX.x > PointY.x) {
                    LastPoint = PointX;
                    mapX += sgnX;
                    result.side = 0;
                }
                else {
                    LastPoint = PointY;
                    mapY += sgnY;
                    result.side = 1;
                }
            }

            if (map.isSolid(mapX, mapY)) {
                result.hit = true;
                break;
            }
        }

        result.mapX   = mapX;
        result.mapY   = mapY;
        result.wallId = map.cellAt(mapX, mapY);

        if (!result.hit) {
            result.depth = kHugeDist;
            return result;
        }

        // perpendicular depth: t where Ray == t * rayDir

        core::Vec2 Ray{LastPoint.x - pos.x, LastPoint.y - pos.y};
        /*
            Ray == rayDir * t == [dir + plane * cameraX] * t
            (Ray, dir) / |dir| == t * ([dir + plane * cameraX], dir) == t * [1 + (plane * cameraX, dir)] == t
            t == Ray / rayDir
        */

        if (result.side == 0) { // wall is parallel to y
            result.depth = Ray.x / rayDir.x; // Ray.x is int
        }
        else { // wall is parallel to x
            result.depth = Ray.y / rayDir.y; // Ray.y is int
        }

        if (result.depth < kEpsilon)
            result.depth = kEpsilon;

        return result;
    }

    void Raycaster::render(const world::Player& player,
                           const world::Map&    map,
                           render::Framebuffer& framebuffer) {
        const int w = framebuffer.width();
        const int h = framebuffer.height();

        for (int x = 0; x < w; ++x) {
            // cameraX [-1, +1]: left to right
            const double cameraX = 2.0 * x / w - 1.0;

            const RayHit rh = castRay(player, map, cameraX);
            
            if (!rh.hit) // inf
                continue;

            // less depth - bigger wall
            const double lineHeight = static_cast<double>(h) / rh.depth;

            int drawStart = static_cast<int>(h * 0.5 - lineHeight * 0.5);
            int drawEnd   = static_cast<int>(h * 0.5 + lineHeight * 0.5);

            if (drawStart < 0) 
                drawStart = 0;
                
            if (drawEnd > h)   
                drawEnd = h;

            const core::ColorRGBA color = wallColor(rh.wallId, rh.side);

            for (int y = drawStart; y < drawEnd; ++y) {
                framebuffer.setPixel(x, y, color);
            }
        }
    }
}
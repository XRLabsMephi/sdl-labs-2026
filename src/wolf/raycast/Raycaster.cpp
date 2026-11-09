#include "wolf/raycast/Raycaster.hpp"
#include "wolf/core/Color.hpp"
#include <cmath>
#include <algorithm>

namespace wolf::raycast {
    RayHit Raycaster::DDA_Ray(const world::Player& player, const world::Map& map, double cameraX) {
        RayHit hitResult;
        double rayDirX = player.direction.x + player.cameraPlane.x * cameraX;
        double rayDirY = player.direction.y + player.cameraPlane.y * cameraX;
        int mapX = static_cast<int>(player.position.x);
        int mapY = static_cast<int>(player.position.y);

        //раст луча
        double deltaDistX, deltaDistY;
        if (rayDirX == 0) {
            deltaDistX = 1e30;
        } else {
            deltaDistX = std::abs(1.0 / rayDirX);
        }
        if (rayDirY == 0) {
            deltaDistY = 1e30;
        } else {
            deltaDistY = std::abs(1.0 / rayDirY);
        }
        double sideDistX = 0, sideDistY = 0;
        int stepX = 0, stepY = 0;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player.position.x - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - player.position.x) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player.position.y - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - player.position.y) * deltaDistY;
        }
        while (!hitResult.hit) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                hitResult.side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                hitResult.side = 1;
            }
            hitResult.wallType = map.getCell(mapX, mapY);
            if (hitResult.wallType > 0) {
                hitResult.hit = true;
            }
        }
        if (hitResult.side == 0) {
            hitResult.wallDist = (sideDistX - deltaDistX);
        } else {
            hitResult.wallDist = (sideDistY - deltaDistY);
        }
        return hitResult;
    }


    void Raycaster::render(render::FrameBuffer& fb, const world::Player& player, const world::Map& map) {
        int width = fb.getWidth();
        int height = fb.getHeight();
        fb.clear(core::Color{50, 50, 50});

        for (int x = 0; x < width; ++x) {
            //проекция
            double cameraX = 2.0 * x / static_cast<double>(width) - 1.0;
            RayHit hit = DDA_Ray(player, map, cameraX);
            if (hit.hit) {
                int lineHeight = static_cast<int>(height / hit.wallDist);
                int drawStart = -lineHeight / 2 + height / 2;
                int drawEnd = lineHeight / 2 + height / 2;
                core::Color wallColor;
                if (hit.wallType == 1) {
                    wallColor = {200, 0, 0};
                } else if (hit.wallType == 2) {
                    wallColor = {0, 200, 0};
                } else {
                    wallColor = {0, 0, 200};
                }
                if (hit.side == 1) {
                    wallColor.r /= 2;
                    wallColor.g /= 2;
                    wallColor.b /= 2;
                }
                fb.drawVerticalLine(x, drawStart, drawEnd, wallColor);
            }
        }
    }
}
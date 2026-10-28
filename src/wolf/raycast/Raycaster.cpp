#include "wolf/raycast/Raycaster.hpp"
#include <cmath>
#include <algorithm>
#include <limits>

namespace wolf {

RayHit Raycaster::castRay(const Map& map, const Vec2& origin, const Vec2& dir) const {
    RayHit result;

    int mapX = static_cast<int>(std::floor(origin.x));
    int mapY = static_cast<int>(std::floor(origin.y));

    // Избежание деления на ноль при строго параллельных лучах
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

    // Итерации алгоритма DDA
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

        // Проверка выхода за границы карты
        if (!map.isInBounds(mapX, mapY)) {
            break;
        }

        int tile = map.get(mapX, mapY);
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
        // Расчёт перпендикулярной дистанции (устранение искажения рыбьего глаза)
        if (hitSide == WallSide::Vertical) {
            result.depth = sideDistX - deltaDistX;
        } else {
            result.depth = sideDistY - deltaDistY;
        }

        // Вычисление точной точки пересечения и координаты текстуры wallU
        result.hitPoint = Vec2{origin.x + dir.x * result.depth, origin.y + dir.y * result.depth};
        double wallX = 0.0;
        if (hitSide == WallSide::Vertical) {
            wallX = origin.y + result.depth * dir.y;
        } else {
            wallX = origin.x + result.depth * dir.x;
        }
        wallX -= std::floor(wallX);
        result.wallU = wallX;
    } else {
        result.depth = std::numeric_limits<double>::infinity();
    }

    return result;
}

void Raycaster::render(FrameBuffer& fb, const Map& map, const Player& player) {
    const int screenWidth = fb.getWidth();
    const int screenHeight = fb.getHeight();

    if (static_cast<int>(m_depthBuffer.size()) != screenWidth) {
        m_depthBuffer.resize(screenWidth);
    }

    for (int x = 0; x < screenWidth; ++x) {
        // Координата столбца камеры: левый = -1, центр = 0, правый = +1
        const double cameraX = 2.0 * x / static_cast<double>(screenWidth) - 1.0;

        // Направление луча
        const Vec2 rayDir = Vec2{
            player.direction.x + player.cameraPlane.x * cameraX,
            player.direction.y + player.cameraPlane.y * cameraX
        };

        RayHit hit = castRay(map, player.position, rayDir);
        m_depthBuffer[x] = hit.depth;
        if (!hit.hit || hit.depth <= 0.0001) {
            continue;
        }

        // Перспективная проекция: высота полосы обратно пропорциональна глубине
        const int lineHeight = static_cast<int>(screenHeight / hit.depth);

        int drawStart = -lineHeight / 2 + screenHeight / 2;
        int drawEnd = lineHeight / 2 + screenHeight / 2;

        const int clampedStart = std::max(0, drawStart);
        const int clampedEnd = std::min(screenHeight - 1, drawEnd);

        Color wallColor = getWallColor(hit.wallId, hit.side);

        for (int y = clampedStart; y <= clampedEnd; ++y) {
            fb.setPixel(x, y, wallColor);
        }
    }
}

const std::vector<double>& Raycaster::getDepthBuffer() const {
    return m_depthBuffer;
}

Color Raycaster::getWallColor(int wallId, WallSide side) const {
    Color base;
    switch (wallId) {
        case 1:  base = Color{200, 50, 50, 255};   break; // Красный
        case 2:  base = Color{50, 200, 50, 255};   break; // Зеленый
        case 3:  base = Color{50, 50, 200, 255};   break; // Синий
        case 4:  base = Color{200, 200, 200, 255}; break; // Белый/Серый
        default: base = Color{180, 180, 50, 255};  break; // Желтый дефолтный
    }

    // Затенение Y-сторон для объемного визуального восприятия формы комнат
    if (side == WallSide::Horizontal) {
        base.r = static_cast<uint8_t>(base.r * 0.7);
        base.g = static_cast<uint8_t>(base.g * 0.7);
        base.b = static_cast<uint8_t>(base.b * 0.7);
    }
    return base;
}

} // namespace wolf

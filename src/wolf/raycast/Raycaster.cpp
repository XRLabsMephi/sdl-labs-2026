#include "wolf/raycast/Raycaster.hpp"

#include "wolf/core/Color.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace wolf {

RayHit Raycaster::castRay(const Map& map, Vec2 origin, Vec2 direction) const {
    if (!std::isfinite(origin.x) || !std::isfinite(origin.y)
        || !std::isfinite(direction.x) || !std::isfinite(direction.y)
        || (direction.x == 0.0 && direction.y == 0.0)
        || origin.x < 0.0 || origin.y < 0.0
        || origin.x >= map.width() || origin.y >= map.height()) {
        return {};
    }
    int mapX = static_cast<int>(std::floor(origin.x));
    int mapY = static_cast<int>(std::floor(origin.y));
    if (map.isWall(mapX, mapY)) {
        return {};
    }

    const double infinity = std::numeric_limits<double>::infinity();
    const double deltaX = direction.x == 0.0 ? infinity : std::abs(1.0 / direction.x);
    const double deltaY = direction.y == 0.0 ? infinity : std::abs(1.0 / direction.y);
    const int stepX = direction.x < 0.0 ? -1 : 1;
    const int stepY = direction.y < 0.0 ? -1 : 1;
    double sideX = (stepX < 0 ? origin.x - mapX : mapX + 1.0 - origin.x) * deltaX;
    double sideY = (stepY < 0 ? origin.y - mapY : mapY + 1.0 - origin.y) * deltaY;

    while (true) {
        WallSide side;
        if (sideX < sideY) {
            sideX += deltaX;
            mapX += stepX;
            side = WallSide::X;
        } else {
            sideY += deltaY;
            mapY += stepY;
            side = WallSide::Y;
        }
        if (!map.contains(mapX, mapY)) {
            return {};
        }
        const int wallId = map.cell(mapX, mapY);
        if (wallId != 0) {
            const double depth = side == WallSide::X
                ? (mapX - origin.x + (1 - stepX) * 0.5) / direction.x
                : (mapY - origin.y + (1 - stepY) * 0.5) / direction.y;
            const double coordinate = side == WallSide::X
                ? origin.y + depth * direction.y : origin.x + depth * direction.x;
            const double wallU = coordinate - std::floor(coordinate);
            return {true, depth, mapX, mapY, wallId, side, wallU};
        }
    }
}

void Raycaster::render(const Map& map, const Player& player, FrameBuffer& frame,
                       const TextureStore* textures) {
    constexpr auto ceiling = rgb(47, 55, 70);
    constexpr auto floor = rgb(76, 70, 60);
    constexpr std::array colors{
        rgb(190, 65, 55), rgb(70, 155, 100), rgb(65, 110, 190),
        rgb(205, 165, 65), rgb(145, 85, 170)
    };
    frame.clear(ceiling);
    frame.fillRows(frame.height() / 2, frame.height(), floor);
    depthBuffer_.assign(frame.width(), std::numeric_limits<double>::infinity());

    for (int x = 0; x < frame.width(); ++x) {
        const double cameraX = 2.0 * x / frame.width() - 1.0;
        const Vec2 direction = player.direction + player.cameraPlane * cameraX;
        const RayHit hit = castRay(map, player.position, direction);
        if (!hit.hit) {
            continue;
        }
        depthBuffer_[x] = hit.depth;
        const double wallHeight = frame.height() / std::max(hit.depth, 1e-6);
        const double top = (frame.height() - wallHeight) * 0.5;
        const double bottom = (frame.height() + wallHeight) * 0.5;
        const int begin = static_cast<int>(std::clamp(top, 0.0, double(frame.height())));
        const int end = static_cast<int>(std::clamp(bottom, 0.0, double(frame.height())));
        if (textures) {
            const Texture& texture = textures->get(hit.wallId);
            int textureX = static_cast<int>(hit.wallU * texture.width);
            if ((hit.side == WallSide::X && direction.x > 0.0)
                || (hit.side == WallSide::Y && direction.y < 0.0)) {
                textureX = texture.width - 1 - textureX;
            }
            for (int y = begin; y < end; ++y) {
                const int textureY = static_cast<int>((y + 0.5 - top) * texture.height / wallHeight);
                auto color = texture.sample(textureX, textureY);
                if (hit.side == WallSide::Y) color = darken(color);
                frame.setPixel(x, y, color);
            }
        } else {
            auto color = colors[(hit.wallId - 1) % colors.size()];
            if (hit.side == WallSide::Y) color = darken(color);
            frame.verticalLine(x, begin, end, color);
        }
    }
}

const std::vector<double>& Raycaster::depthBuffer() const { return depthBuffer_; }

}

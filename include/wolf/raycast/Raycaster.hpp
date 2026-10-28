#pragma once

#include "wolf/raycast/RayHit.hpp"
#include "wolf/core/Vec2.hpp"
#include "wolf/core/Color.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include <vector>

namespace wolf {

class Raycaster {
public:
    Raycaster() = default;

    // Запуск отдельного DDA-луча
    RayHit castRay(const Map& map, const Vec2& origin, const Vec2& dir) const;

    // Рендеринг стен сцены в FrameBuffer
    void render(FrameBuffer& fb, const Map& map, const Player& player);

    // Буфер глубины стен
    const std::vector<double>& getDepthBuffer() const;

private:
    Color getWallColor(int wallId, WallSide side) const;
    std::vector<double> m_depthBuffer;
};

} // namespace wolf

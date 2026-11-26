#pragma once

#include <vector>

#include "wolf/core/Color.hpp"
#include "wolf/core/Vec2.hpp"
#include "wolf/raycast/RayHit.hpp"
#include "wolf/render/Fog.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace wolf {

class Raycaster {
public:
    Raycaster() = default;

    // Запуск отдельного DDA-луча
    RayHit castRay(const Map& map, const Vec2& origin, const Vec2& dir) const;

    // Рендеринг стен сцены в FrameBuffer
    void render(FrameBuffer& fb, const Map& map, const Player& player);

    // Текстурированные стены, туман и буфер глубины
    void render(
        FrameBuffer& frame,
        const Map& map,
        const Player& player,
        const TextureStore& textures,
        const Fog& fog
    );

    // Буфер глубины стен
    const std::vector<double>& getDepthBuffer() const;

private:
    Color getWallColor(int wallId, WallSide side) const;
    std::vector<double> m_depthBuffer;
};

} // namespace wolf

#pragma once

#include "raycast/RayHit.hpp"
#include "render/FrameBuffer.hpp"
#include "world/Map.hpp"
#include "world/Player.hpp"

/*
Raycaster - тип, пускающий лучи камеры и определяющий свойства стены. Математически отрисовывает картину в FrameBuffer.
*/
class RayCaster {
    std::vector<double> depth_buffer_; //список глубин

    public:
        RayCaster() = default;

        [[nodiscard]] RayHit cast_ray(const Player& player, const Map& map, double cameraX) const; //пуск луча по алгоритму DDA
        void render(const Player& player, const Map& map, FrameBuffer& buffer); //отрисовка
        [[nodiscard]] const std::vector<double>& get_depth() const; //геттер глубин
};

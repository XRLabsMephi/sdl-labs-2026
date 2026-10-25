#pragma once
#include <iostream>
#include <stdexcept>
#include <raycast/RayHit.h>
#include <core/Vec2.h>
#include <world/Map.h>
#include <render/FrameBuffer.h>
#include <world/Player.h>

namespace raycast{
    using Vector2 = core::Vector2;
    //TODO documentation
    class RayCaster{
    public:
        //возможно стоит добавить во frame_buffer - геттеры для получения frame_width и frame_height
        /**@param frame_width the width of frame, should be more than 1**/
        RayCaster(int frame_width, int frame_height, const world::Player& player, const world::Map& map, render::FrameBuffer& frame_buffer)
            :
            player_(player),
            map_(map),
            frame_buffer_(frame_buffer),
            frame_width_(frame_width),
            frame_height_(frame_height)
        {if (frame_width_ <= 1 || frame_height_ <= 0){
            //надо принять соглашение об исключениях и логгировании
            throw std::invalid_argument("Invalid frame resolution");
        }}
        /**
        * Cast rays for each column of screen.
        * Based on data from class Map from "Map.h".
        * Set obtained information to FrameBuffer from "FrameBuffer.h"
        */
        void DoRayCasting() const;
        //по сути он const, хотя и меняет состояние frame_buffer

    private:
        const world::Player& player_;
        const world::Map& map_;
        render::FrameBuffer& frame_buffer_;
        int frame_width_, frame_height_;

        static constexpr double angle_eps = 1e-7f;
        static constexpr double pi = 3.1415926536;
        //Cast a single ray with given nonzero direction
        RayHit CastRay(Vector2 direction) const;
    };
}

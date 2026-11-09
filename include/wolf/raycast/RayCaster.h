#pragma once
#include <stdexcept>
#include <raycast/RayHit.h>
#include <core/Vec2.h>
#include <core/Color.h>
#include <world/Map.h>
#include <render/FrameBuffer.h>
#include <world/Player.h>
#include <render/TextureStorage.h>
#include <render/Fog.h>

namespace raycast{
    namespace settings{
        inline core::Color WALL_COLOR = core::Color(255, 175, 119);
        inline core::Color GROUND_COLOR = core::Color::lerp(core::Color::black, core::Color::white, 255 * 3 / 10);
        inline core::Color SKY_COLOR = core::Color(170, 142, 224);

        //constexpr double vertical_field_of_view = 0.0; //the angle of view in vertical axe. Interchangeably with SC_D
        constexpr double SC_D = 1.0; //distance between camera and virtual screen

        inline double minimal_brightness = 0.6;
        inline double darkening_distance = 30.0;

        inline bool fish_eye = false;
    }


    using Vector2 = core::Vector2;
    /**@brief The class that implements casting rays and set drawing parameters*/
    enum Effects{
        None,
        FishEye
    };

    class RayCaster{
    public:
        //возможно стоит добавить во frame_buffer - геттеры для получения frame_width и frame_height
        /**@param frame_width the width of frame, should be more than 1
         *@param frame_height
         *@param player
         *@param map
         *@param frame_buffer
         *@param storage**/
        RayCaster(const world::Player& player, const world::Map& map, render::FrameBuffer& frame_buffer,
            const render::TextureStorage& storage)
            :
            player_(player),
            map_(map),
            frame_buffer_(frame_buffer),
            storage_(storage),
            frame_width_(static_cast<int>(frame_buffer_.width())),
            frame_height_(static_cast<int>(frame_buffer_.height())),
            depth_buffer_(frame_width_)
        {if (frame_width_ <= 1 || frame_height_ <= 0){
            throw std::invalid_argument("Invalid frame resolution");
        }}
        /**
        * Cast rays for each column of screen.
        * Based on data from class Map from "Map.h".
        * Set obtained information to FrameBuffer from "FrameBuffer.h"
        */
        void renderScene();

        /**@return const ref to buffer of depths*/
        [[nodiscard]] std::vector<double>& getDepthBuffer(){return depth_buffer_;};

        void set_fog_params(core::Color fog_color, float min_fog_depth, float max_fog_depth) {
            fog_ = Fog(min_fog_depth, max_fog_depth, fog_color);
        }


    private:
        const world::Player& player_;
        const world::Map& map_;
        render::FrameBuffer& frame_buffer_;
        const render::TextureStorage &storage_;
        int frame_width_, frame_height_;
        Fog fog_;
        std::vector<double> depth_buffer_;


        static constexpr double angle_eps = 1e-7f;
        static constexpr double eps = 1e-8f;
        static constexpr double pi = 3.1415926536;

        RayHit ray_cast_hit;
        //Cast a single ray with given nonzero direction and save information in array ray_cast_hits
        void castRay(Vector2 direction);

        //draw column in FrameBuffer
        void drawByRayHit(const std::pair<double, double>& cos_vertical_horizontal,
                          double depth, int column) const;
    };
}

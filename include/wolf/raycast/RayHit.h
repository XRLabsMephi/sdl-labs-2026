#pragma once
#include <core/Vec2.h>
namespace raycast{
    /**@brief storage for the side of hit\n
         *@c None = -1\n
         *@c Vertical = 0 : the side that is parallel to ordinate\n
         *@c Horizontal = 1 : the side that is parallel to abscissa**/
    enum class IntersectionSide{None = -1, Vertical = 0, Horizontal = 1};
    using Vector2 = core::Vector2;
    //Data class for ray hit
    struct RayHit {
        public:
            RayHit() = default;
        /**
         * @param has_hit has hit (if false any other parameters has no sense)
         * @param t_distance the length of ray way before hit in t-measuring
         * @param coord coordinate of hit
         * @param wall_id id of wall
         * @param face_id the index of wall's face, where ray was hit
         * @param wall_u the hit position relative to wall
         **/
            RayHit(bool has_hit, double t_distance, Vector2 coord, int wall_id, IntersectionSide face_id, double wall_u)
                :
                has_hit_(has_hit),
                wall_id_(wall_id),
                face_index_(face_id),
                t_distance_(t_distance),
                wall_u_(wall_u),
                coordinate_(coord)
                 {}
        bool has_hit_ = false; //has hit (if false other parameters make no sense)
        int wall_id_ = -1; //id of wall
        IntersectionSide face_index_ = IntersectionSide::None; //the index of wall's face, where ray was hit
        double t_distance_ = std::numeric_limits<double>::infinity(); //the length of ray way before hit
        double wall_u_ = 0.0; //the hit position relative to wall
        Vector2 coordinate_ {0.0, 0.0}; //coordinate of hit
    };
}

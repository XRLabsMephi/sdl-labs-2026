#pragma once
#include <core/Vec2.h>

namespace raycast{
    /**@brief storage for the side of hit\n
         *@c None = -1\n
         *@c Vertical = 0 : the side that is parallel to ordinate\n
         *@c Horizontal = 1 : the side that is parallel to abscissa**/
    enum class IntersectionSide{None = -1, Vertical = 0, Horizontal = 1}; // возможно стоит переименовать
    using Vector2 = core::Vector2;
    //Data class for ray hit
    struct RayHit {
        public:
            RayHit() = default;
        /**
         * @param has_hit has hit (if false any other parameters has no sense)
         * @param distance the length of ray way before hit
         * @param coord coordinate of hit
         * @param wall_id id of wall
         * @param face_id the index of wall's face, where ray was hit
         * @param hit_cos2 the cos^2 of the hit angle (using for smooth shadows changes)
         * @param coor_rel_wall the hit position relative to wall
         **/
            RayHit(bool has_hit, double distance, Vector2 coord, int wall_id, IntersectionSide face_id, double hit_cos2, int coor_rel_wall)
                :
                has_hit_(has_hit),
                distance_(distance),
                coordinate_(coord),
                wall_id_(wall_id),
                face_index_(face_id),
                hit_cos2_(hit_cos2),
                coordinate_relate_wall_(coor_rel_wall) {}
        bool has_hit_ = false; //has hit (if false other parameters make no sense)
        double distance_ = 0.0; //the length of ray way before hit
        Vector2 coordinate_ {0.0, 0.0}; //coordinate of hit
        int wall_id_ = -1; //id of wall
        IntersectionSide face_index_ = IntersectionSide::None; //the index of wall's face, where ray was hit
        double hit_cos2_ = 0.0; //the cos^2 of the hit angle relative to normal (using for smooth shadows changes)
        //for lab2
        int coordinate_relate_wall_ = 0.0; //the hit position relative to wall
    };
}

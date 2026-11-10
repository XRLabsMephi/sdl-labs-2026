#include <raycast/RayCaster.h>
#include <render/FrameBuffer.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <cassert>

using namespace raycast;
using namespace raycast::settings;

void RayCaster::drawByRayHit(const std::pair<double, double>& cos_vertical_horizontal, double depth,
                             double tan_half_fov, int column) const{
    const int W_H = frame_height_; // the height of wall in pixels
    const int P_H = player_.getEyeHeight(); // the height of player in pixels

    //projection of distance on camera axe (using scalar product and the fact, that camera_direction is normalized)


    //calculating for shadows
    double brightness_procent_cos = ray_cast_hit.face_index_ == IntersectionSide::Vertical ?
                                cos_vertical_horizontal.first : cos_vertical_horizontal.second;
    //for smooth changing
    double brightness_procent_depth = std::max(0.0, (1.0 - (depth/darkening_distance)));
    double brightness_procent = (std::max(0.5, brightness_procent_cos)) * brightness_procent_depth;
    double brightness = (1.0-minimal_brightness) * brightness_procent + minimal_brightness;

    Uint8 fog_factor = fog_.factor(depth);
    auto get_color_with_fog = [this, &fog_factor](core::Color color)
    {return core::Color::fromBits(fog_.applyFog(static_cast<Uint32>(color), fog_factor));};

    //draw sky and ground
    frame_buffer_.vline(column, 0, frame_height_ - P_H, SKY_COLOR);
    frame_buffer_.vline(column, frame_height_ - P_H + 1, frame_height_, GROUND_COLOR);


    const RayHit& ray_hit = ray_cast_hit;
    bool with_texture = ray_hit.wall_id_ >= 2;
    const render::Texture& texture = storage_.getOrEmpty(static_cast<size_t>(ray_cast_hit.wall_id_));
    if (&texture == &render::Texture::empty){
        with_texture = false;
    }

    if (ray_hit.has_hit_){
        const double SC_D = (frame_width_ - 1)/(2 * tan_half_fov * W_H); //distance between camera and virtual screen
        const double SC_RAY_DEPTH_rel = SC_D/depth;
        int y0 = static_cast<int>(P_H * (1 - SC_RAY_DEPTH_rel)); //bottom border of wall projection in pixels(can be negative)
        int y1 = static_cast<int>(P_H + SC_RAY_DEPTH_rel * (W_H - P_H)); //top border of wall projection in pixels(can be more than frame_height_)
        const Uint32* tex_column_ptr = texture.column(static_cast<int>(ray_cast_hit.wall_u_ * texture.width));
        int length_of_wall = y1 - y0 + 1;
        //draw wall
        frame_buffer_.vline(column, frame_height_ - y1, frame_height_ - y0, get_color_with_fog(WALL_COLOR));
        if (with_texture){
            double step = static_cast<double>(texture.height)/static_cast<double>(length_of_wall);
            auto castToFixedFloat = [](double value){double int_value, frac_value =  std::modf(value, &int_value);
                return (static_cast<int>(int_value) << 16) | static_cast<int>(frac_value * (1 << 16));};
            //int tex_start = castToFixedFloat(std::max(0, y1 - frame_height_) * step); //frameBuffer implements this
            int tex_start = 0;
            int tex_step = castToFixedFloat(step); //cast to 16.16 format
            render::DrawCallParams params = {tex_column_ptr, tex_start, tex_step, depth, static_cast<Uint8>(255 * brightness)};
            frame_buffer_.vline<false>(column, frame_height_ - y0, frame_height_ - y1, params);
        }
    }
}


void RayCaster::renderScene(){

    Vector2 camera_direction = player_.getDirection();
    double field_of_view = player_.getAngle(); // the angle of view in radians (вопрос какой угол)

    //check Player contract
    assert(!(camera_direction.x == 0.0 && camera_direction.y == 0.0) && "[Error] Zero ray direction.");
    assert(std::abs(camera_direction.length() - 1.0) < eps  && "[Error] Not normalized vector of camera direction.");
    assert(!(std::fabs(field_of_view) > pi - angle_eps) && "[Error] Field-of-view angle out of range.");

    double tan_half_fov = tan(field_of_view / 2.0f);

    //Further, we turn camera direction by -pi/2 to get orthogonal vector and transform it to necessary for current fov length

    // the vector of half of virtual screen (where image is projected)
    Vector2 plane = Vector2{camera_direction.y, -camera_direction.x} * tan_half_fov;

    //TODO реализовать другие варианты для запуска лучей(в середины отрезков, по равным углам ...)

    // the step vector
    Vector2 shift = plane * (2.0 / static_cast<double>(frame_width_ - 1));

    //show from which position of segment casting start
    constexpr double start_fraction = 0.0;
    Vector2 ray_direction = camera_direction - plane + shift * start_fraction; // the first direction

    //the cos of the angle between camera direction and corresponding side of cell normal
    std::pair<double, double> cos_vertical_horizontal = {std::abs(camera_direction.x), std::abs(camera_direction.y)};
    //сейчас реализация такова, что делим на w - 1 отрезок и берем границы в качестве векторов направления
    /*возможно она будет изменена или просто будут добавлены дополнительно расчет
     *на основе равных углов или привязки к серединам отрезков*/
    for (int xi = 0; xi < frame_width_; ++xi, ray_direction += shift){ //calculate next direction of ray
        castRay(ray_direction);
        double depth;
        if (!fish_eye){depth = ray_cast_hit.t_distance_ * ray_direction.dot(camera_direction);}
        else {depth = ray_cast_hit.t_distance_ * ray_direction.length();}
        depth_buffer_[xi] = depth;
        drawByRayHit(cos_vertical_horizontal, depth, tan_half_fov, xi);
    }


}

void RayCaster::castRay(Vector2 ray_direction) {
    Vector2 ray_coordinate = player_.getPosition();

    const auto& [ray_dir_x, ray_dir_y] = ray_direction;
    const auto& [x, y] = ray_coordinate;

    //preprocessing
    //we count t-parameters in another units of measurement. De-facto one t is length of ray_direction

    //from the similarity of triangles
    const double tdx = 1.0f / std::fabs(ray_dir_x); //shift through ray-axis in t-measuring when changing x for one //if denominator zero will be inf
    const double tdy = 1.0f / std::fabs(ray_dir_y); //shift through ray-axis in t-measuring when changing y for one//if denominator zero will be inf

    const int sign_dx = (ray_dir_x == 0.0) ? 0 : ray_dir_x > 0.0 ? 1 : -1;
    const int sign_dy = (ray_dir_y == 0.0) ? 0 : ray_dir_y > 0.0 ? 1 : -1;

    double double_cell_x = std::floor(x), double_cell_y = std::floor(y); //the coordinate of cell where x and y is

    IntersectionSide intersection_side = IntersectionSide::None; //the cell side that was hit
    if (x == double_cell_x){intersection_side = IntersectionSide::Vertical;}
    if (y == double_cell_y){intersection_side = IntersectionSide::Horizontal;}

    if (sign_dx < 0 && x == double_cell_x) {--double_cell_x;}
    if (sign_dy < 0 && y == double_cell_y) {--double_cell_y;}

    const auto rx = double_cell_x + (sign_dx > 0 ? 1.0 : 0.0) - x;
    const double ry = double_cell_y + (sign_dy > 0 ? 1.0 : 0.0) - y;

    //coordinate in ray-axe in t-measuring of closest not processed x-border
    double closest_t_x = std::numeric_limits<double>::infinity();
    if (ray_dir_x != 0.0){
        //from the similarity of triangles
        closest_t_x = rx/ray_dir_x;
    }
    //coordinate in ray-axe in t-measuring of closest not processed y-border
    double closest_t_y = std::numeric_limits<double>::infinity();
    if (ray_dir_y != 0.0){
        //from the similarity of triangles
        closest_t_y = ry/ray_dir_y;
    }

    double t_inter = 0.0; //coordinate in ray-axe in t-measuring of the last intersection_side

    int cell_x = static_cast<int>(double_cell_x), cell_y = static_cast<int>(double_cell_y); //start cell coordinates

    //variables for transparency
    //main loop
    //calculating the next cell and check is there wall
    //TODO более подробно расписать работу алгоритма
    while (true){
        if (map_.isWall(cell_x, cell_y)){
            int wall_id = map_.getCell(cell_x, cell_y);
            if (wall_id == -1){
                ray_cast_hit = RayHit{};
                break;
            }
            Vector2 coordinates = ray_coordinate + ray_direction * t_inter;
            ray_cast_hit = RayHit(
                true, t_inter,
                coordinates, wall_id, intersection_side,
                intersection_side == IntersectionSide::Horizontal ?
                coordinates.x - std::floor(coordinates.x): coordinates.y - std::floor(coordinates.y)
            );
            break;
        }
        if (closest_t_x > closest_t_y){
            cell_y += sign_dy;
            t_inter = closest_t_y;
            closest_t_y += tdy;
            intersection_side = IntersectionSide::Horizontal;
        }
        else if (closest_t_x < closest_t_y){
            cell_x += sign_dx;
            t_inter = closest_t_x;
            closest_t_x += tdx;
            intersection_side = IntersectionSide::Vertical;
        }
        else
        {
            if (map_.isWall(cell_x + sign_dx, cell_y))
            {
                closest_t_x = -1.0f; //terminating
                continue;
            }
            if (map_.isWall(cell_x, cell_y + sign_dy))
            {
                closest_t_y = -1.0f; //terminating
                continue;
            }
            cell_x += sign_dx;
            cell_y += sign_dy;
            closest_t_x += tdx;
            closest_t_y += tdy;
        }
    }
}

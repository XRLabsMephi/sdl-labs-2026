#include <raycast/Raycaster.h>
#include <core/Color.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <cassert>
#include <array>

using namespace raycast;

enum Colors{
    SKY = -1,
    GROUND = 0,
    WALL1 = 1,
    WALL2 = 2,
};

core::Color W1_COLOR = core::Color(255, 175, 119);
core::Color W2_COLOR = core::Color(181, 58, 102);
core::Color wall_colors[2] = {W1_COLOR, W2_COLOR};

void RayCaster::drawByRayHit(const Vector2& ray_direction, const Vector2& camera_direction, const std::pair<double, double>& cos_vertical_horizontal, int column) const{
    /////////////////////////////////////////// parameters
    //TODO как то организовать ввод и получение параметров

    //ground parameters
    core::Color GROUND_COLOR = core::Color::lerp(core::Color::black, core::Color::white, 30);

    //sky parameters
    core::Color SKY_COLOR = core::Color(170, 142, 224);

    //wall parameters
    const int W_H = frame_height_; // the height of wall in pixels

    //other parameters
    constexpr double W_P_rel = 2; // the relation between wall and player height
    constexpr double P_W_rel = 1.0 / W_P_rel; // the relation between player and wall height
    const int P_H = static_cast<int>(W_H * P_W_rel); // the height of player in pixels

    constexpr double SC_D = 1.0; //distance between camera and virtual screen

    double minimal_brightness = 0.6;
    double darkening_distance = 30.0;
    //texture parameters
    const int texture_width = 512;
    //constexpr double vertical_field_of_view = 0.0; //the angle of view in vertical axe. Interchangeably with SC_D

    ///////////////////////////////////////////

    auto get_draw_params = [this, &minimal_brightness, &darkening_distance, &cos_vertical_horizontal, &GROUND_COLOR, &SKY_COLOR]
    (Colors color_id, double wall_u = 0, double depth = 0, bool with_texture = false) -> render::DrawCallParams{
        core::Color color;
        double brightness = 1.0;
        switch (color_id){
            case Colors::WALL1:
            case Colors::WALL2:{
                color = wall_colors[color_id - 1];
                //calculating for shadows
                double brightness_procent_cos = ray_cast_hit.face_index_ == IntersectionSide::Vertical ?
                                            cos_vertical_horizontal.first : cos_vertical_horizontal.second;
                //for smooth changing
                double brightness_procent_depth = std::max(0.0, (1.0 - (depth/darkening_distance)));
                double brightness_procent = (std::max(0.5, brightness_procent_cos)) * brightness_procent_depth;
                brightness = (1.0-minimal_brightness) * brightness_procent + minimal_brightness;
                break;
                }
            case Colors::GROUND:
                color = GROUND_COLOR; break;
            case Colors::SKY:
                color = SKY_COLOR; break;
            default: break;
        }

        color.r_ = static_cast<Uint8>(color.r_ * brightness);
        color.g_ = static_cast<Uint8>(color.g_ * brightness);
        color.b_ = static_cast<Uint8>(color.b_ * brightness);
        if (!with_texture){
            return {.tex_id = -1, .tex_x = -1, .alpha_canal = color.a_, .brightness = brightness, .color = color};
        }
        return {.tex_id = ray_cast_hit.wall_id_, .tex_x = static_cast<int>(wall_u * texture_width)
            , .alpha_canal = -1, .brightness = brightness, .color = core::Color{}};
    };
    //draw sky and ground
    frame_buffer_.vline(column, 0, frame_height_ - P_H, get_draw_params(Colors::SKY));
    frame_buffer_.vline(column, frame_height_ - P_H + 1, frame_height_, get_draw_params(Colors::GROUND));

    const RayHit& ray_hit = ray_cast_hit;
    if (ray_hit.has_hit_){
        double depth = ray_hit.t_distance_ * (ray_direction.x * camera_direction.x + ray_direction.y * camera_direction.y);
        const double SC_RAY_DEPTH_rel = SC_D/depth;
        int y0 = static_cast<int>(P_H * (1 - SC_RAY_DEPTH_rel)); //bottom border of wall projection in pixels(can be negative)
        int y1 = static_cast<int>(P_H + SC_RAY_DEPTH_rel * (W_H - P_H)); //top border of wall projection in pixels(can be more than frame_height_)
        frame_buffer_.vline(column, frame_height_ - y1, frame_height_ - y0, get_draw_params(static_cast<Colors>(ray_hit.wall_id_), ray_hit.wall_u_, depth, false));
        //frame_buffer_.vline(column, frame_height_ - y1, frame_height_ - y0, get_draw_params(static_cast<Colors>(ray_hit.wall_id_), ray_hit.wall_u_, depth, true));
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
        drawByRayHit(ray_direction, camera_direction, cos_vertical_horizontal, xi);
    }


}

void RayCaster::castRay(Vector2 ray_direction) {
    Vector2 ray_coordinate = player_.getPosition();

    auto& [ray_dir_x, ray_dir_y] = ray_direction;
    auto& [x, y] = ray_coordinate;

    //preprocessing
    //we count t-parameters in another units of measurement. De-facto one t is length of ray_direction

    //from the similarity of triangles
    double tdx = 1.0f / std::fabs(ray_dir_x); //shift through ray-axis in t-measuring when changing x for one //if denominator zero will be inf
    double tdy = 1.0f / std::fabs(ray_dir_y); //shift through ray-axis in t-measuring when changing y for one//if denominator zero will be inf

    int sign_dx = (ray_dir_x == 0.0) ? 0 : ray_dir_x > 0.0 ? 1 : -1;
    int sign_dy = (ray_dir_y == 0.0) ? 0 : ray_dir_y > 0.0 ? 1 : -1;

    double double_cell_x = std::floor(x), double_cell_y = std::floor(y); //the coordinate of cell where x and y is

    IntersectionSide intersection_side = IntersectionSide::None; //the cell side that was hit
    if (x == double_cell_x){intersection_side = IntersectionSide::Vertical;}
    if (y == double_cell_y){intersection_side = IntersectionSide::Horizontal;}

    if (sign_dx < 0 && x == double_cell_x) {--double_cell_x;}
    if (sign_dy < 0 && y == double_cell_y) {--double_cell_y;}

    const double rx = double_cell_x + (sign_dx > 0 ? 1.0 : 0.0) - x;
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
                intersection_side == IntersectionSide::Vertical ?
                coordinates.x - std::floor(coordinates.x): coordinates.y - std::floor(coordinates.y)
            );
            break;
        }
        if (closest_t_x > closest_t_y){
            cell_y += sign_dy;
            t_inter = closest_t_y;
            closest_t_y += tdy;
            intersection_side = IntersectionSide::Horizontal;
            continue;
        }
        else{
            cell_x += sign_dx;
            t_inter = closest_t_x;
            closest_t_x += tdx;
            intersection_side = IntersectionSide::Vertical;
        }
        // if ray pass next to corner, it intersects one cell before, firstly vertical, secondly horizontal
        // возможно стоит сделать отдельную проверку на обе
        /*else
        {
            if (map_.isWall(cell_x + sign_dx, cell_y))
            {
                closest_t_x = -1.0f; //terminating
                continue;
            }
            else if (map_.isWall(cell_x, cell_y + sign_dy))
            {
                closest_t_y = -1.0f; //
                continue;
            }
            cell_x += sign_dx;
            cell_y += sign_dy;
            closest_t_x += tdx;
            closest_t_y += tdy;
        }*/
    }
}

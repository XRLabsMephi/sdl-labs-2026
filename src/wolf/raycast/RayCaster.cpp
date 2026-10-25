#include <raycast/Raycaster.h>
#include <core/Color.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <cassert>

using namespace raycast;


void RayCaster::DrawByRayHit(const RayHit& ray_hit, double depth, int column) const{
    /////////////////////////////////////////// parameters
    //sky parameters
    core::Color SKY_COLOR = core::Color::blue;

    //ground parameters
    core::Color GROUND_COLOR = core::Color::lerp(core::Color::black, core::Color::white, 30);

    //wall parameters
    const int W_H = frame_height_; // the height of wall in pixels
    core::Color W_COLOR = core::Color::red;

    //other parameters
    constexpr double W_P_rel = 2.0; // the relation beetween player and wall height
    constexpr double P_W_rel = 1.0 / W_P_rel; // the relation beetween wall and player height
    const int P_H = static_cast<int>(W_H * P_W_rel); // the height of player in pixels

    constexpr double SC_D = 1.0; //distance beetween camera and virtual screen
    //constexpr double vertical_field_of_view = 0.0; //the angle of view in vertical axe. Interchangeably with SC_D

    ///////////////////////////////////////////

    const double SC_RAY_DEPTH_rel = SC_D/depth;

    int y0 = static_cast<int>(P_H * (1 - SC_RAY_DEPTH_rel)); //bottom border of wall projection in pixels(can be negative)
    int y1 = static_cast<int>(P_H + SC_RAY_DEPTH_rel * (W_H - P_H)); //top border of wall projection in pixels(can be more than frame_height_)

    auto get_draw_params = [&ray_hit, depth, W_COLOR, SKY_COLOR, GROUND_COLOR](int wall_id) -> render::DrawCallParams{
        core::Color color;
        switch (wall_id){
            case 0: color = W_COLOR; break;
            case 1: color = GROUND_COLOR; break;
            case 2: color = SKY_COLOR; break;
            default: break;
        }
        double brightness = ray_hit.hit_cos2_; // the brightness in [0, 1]
        color.r_ = static_cast<Uint8>(color.r_ * brightness);
        color.g_ = static_cast<Uint8>(color.g_ * brightness);
        color.b_ = static_cast<Uint8>(color.b_ * brightness);
        return {.tex_id = ray_hit.wall_id_, .tex_x = -1, .depth = depth, .color = color};
    };

    frame_buffer_.vline(column, y0, y1, get_draw_params(ray_hit.wall_id_));
}


void RayCaster::DoRayCasting() const{

    Vector2 camera_direction = player_.getDirection();
    double field_of_view = player_.getAngle(); // the angle of view in radians (вопрос какой угол)

    //check Player contract
    assert(!(camera_direction.x == 0.0 && camera_direction.y == 0.0) && "[Error] Zero ray direction.");
    assert(std::abs(camera_direction.length() - 1.0) < eps  && "[Error] Not normalized vector of camera direction.");
    assert(!(std::fabs(field_of_view) > pi - angle_eps) && "[Error] Field-of-view angle out of range.");

    double tan_half_fov = tan(field_of_view / 2.0f);

    //Further, we turn camera direction by -pi/2 to get orthogonal vector and transform it to necessary for current fov length
    // the vector of half of virtual screen (where image is projected)
    Vector2 plane = {camera_direction.y * tan_half_fov, -camera_direction.x * tan_half_fov};

    // the step vector
    Vector2 shift = {2.0f * plane.x / static_cast<double>(frame_width_ - 1), 2.0f * plane.y / static_cast<double>(frame_width_ - 1)};
    Vector2 ray_direction = camera_direction - plane; // the first direction

    //сейчас реализация такова, что делим на w - 1 отрезок и берем границы в качестве векторов направления
    /*возможно она будет изменена или просто будут добавлены дополнительно расчет
     *на основе равных углов или привязки к серединам отрезков*/
    for (int xi = 0; xi < frame_width_; ++xi, ray_direction += shift) //calculate next direction of ray
    {

        // можно оптимизировать предпроцессинг и избавиться от корней в теории
        //normalization
        double len = ray_direction.length();
        Vector2 normal_ray_direction;
        normal_ray_direction = {.x = ray_direction.x / len, .y = ray_direction.y / len};

        RayHit ray_hit = CastRay(normal_ray_direction);

        //projection of ray vector way on camera vector
        double depth = ray_hit.distance_ * (normal_ray_direction.x * camera_direction.x + normal_ray_direction.y * camera_direction.y);

        DrawByRayHit(ray_hit, depth, xi);
    }


}

RayHit RayCaster::CastRay(Vector2 ray_direction) const{
    double distance = 0;
    Vector2 ray_coordinate = player_.getPosition();

    auto [ray_dir_x, ray_dir_y] = ray_direction;
    auto [x, y] = ray_coordinate;

    //preprocessing

    double tdx = 1.0f / std::fabs(ray_dir_x); //shift through ray-axis when changing x //if denominator zero will be inf
    double tdy = 1.0f / std::fabs(ray_dir_y); //shift through ray-axis when changing y //if denominator zero will be inf

    int sign_dx = (ray_dir_x == 0.0) ? 0 : ray_dir_x > 0.0 ? 1 : -1;
    int sign_dy = (ray_dir_y == 0.0) ? 0 : ray_dir_y > 0.0 ? 1 : -1;

    double double_cell_x = std::floor(x), double_cell_y = std::floor(y); //the coordinate of cell where x and y is

    IntersectionSide intersection_side = IntersectionSide::None; //the cell side that was hit
    if (x == double_cell_x){intersection_side = IntersectionSide::Vertical;}
    if (y == double_cell_y){intersection_side = IntersectionSide::Horizontal;}

    if (sign_dx < 0 && x == double_cell_x) {--double_cell_x;}
    if (sign_dy < 0 && y == double_cell_y) {--double_cell_y;}

    double rx = double_cell_x + (sign_dx > 0 ? 1.0 : 0.0) - x;
    double ry = double_cell_y + (sign_dy > 0 ? 1.0 : 0.0) - y;

    //coordinate in ray-axe closest not processed x-border
    double closest_t_x = std::numeric_limits<double>::infinity();
    if (ray_dir_x != 0.0){
        double yl = rx * (ray_dir_y / ray_dir_x);
        closest_t_x= std::sqrt(rx * rx + yl * yl);
    }

    //coordinate in ray-axe closest not processed y-border
    double closest_t_y = std::numeric_limits<double>::infinity();
    if (ray_dir_y != 0.0){
        double xl = ry * (ray_dir_x / ray_dir_y);
        closest_t_y = std::sqrt(ry * ry + xl * xl);
    }

    double t_inter = 0.0; //coordinate in ray-axe of the last intersection_side

    int cell_x = static_cast<int>(double_cell_x), cell_y = static_cast<int>(double_cell_y); //start cell coordinates
    //main loop
    //calculating the next cell and check is there wall
    //TODO более подробно расписаро работу алгоритма
    while (!map_.isWall(cell_x, cell_y)){
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
    distance = t_inter;
    int wall_id = map_.getCell(cell_x, cell_y);

    //count angles with cos^2

    //cos^2 of the angle through x-axe
    double dir_square_cos_x = ray_dir_x * ray_dir_x / (ray_dir_x * ray_dir_x + ray_dir_y * ray_dir_y);
    //cos^2 of the hit angle
    double hit_square_cos = intersection_side == IntersectionSide::Vertical ? dir_square_cos_x: 1.0f - dir_square_cos_x;

    ray_coordinate.x += ray_dir_x * t_inter;
    ray_coordinate.y += ray_dir_y * t_inter;
    //в данный момент не ясно как обрабатывать и получать rel_wall
    RayHit rayHit(true, distance, ray_coordinate, wall_id, intersection_side, hit_square_cos, 0);
    return rayHit;
}


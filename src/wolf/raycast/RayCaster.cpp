#include <raycast/Raycaster.h>
#include <iostream>
#include <cmath>
#include <limits>

using namespace raycast;

void RayCaster::DoRayCasting() const{
    Vector2 camera_direction = player_.getDirection();

    //возможно стоит заменить на assert
    if (camera_direction.x == 0.0 && camera_direction.y == 0.0){
        std::cerr << "[Error] Zero ray direction." << "\n";
        return;
    }

    double field_of_view = player_.getAngle(); // the angle of view in radians (вопрос какой угол)
    //validate angle to prevent inf tan
    if (std::fabs(field_of_view) > pi - angle_eps){
        std::cerr << "[Error] Field-of-view angle out of range." << "\n";
        return;
    }

    double tan_half_fov = tan(field_of_view / 2.0f);

    //Further, we turn camera direction by -pi/2 to get orthogonal vector and transform it to necessary for current fov length

    // the vector of half of virtual screen (where image is projected)
    Vector2 plane = {camera_direction.y * tan_half_fov, -camera_direction.x * tan_half_fov};

    // the step vector
    Vector2 shift = {2.0f * plane.x / static_cast<double>(frame_width_ - 1), 2.0f * plane.y / static_cast<double>(frame_width_ - 1)};
    // the first direction
    Vector2 ray_direction = camera_direction - plane;

    //сейчас реализация такова, что делим на w - 1 отрезок и берем границы в качестве векторов направления
    /*возможно она будет изменена или просто будут добавлены дополнительно расчет
     *на основе равных углов или привязки к серединам отрезков*/
    for (int i = 0; i < frame_width_; ++i)
    {
        RayHit ray_hit = CastRay(ray_direction);
        ray_direction += shift; //calculate next direction of ray

        //TODO сделать взаимодействия с framebuffer(в т.ч расчёт цвета, теней, depth=проекцию distance на вектор камеры), высоту земли, стен и т.д)
    }
}

RayHit RayCaster::CastRay(Vector2 ray_direction) const{
    double distance = 0;
    Vector2 ray_coordinate = player_.getPosition();

    // можно оптимизировать предпроцессинг и избавиться от корней в теории
    //normalization

    double len = ray_direction.length();
    if (std::isinf(len) || len == 0.0){
        std::cerr << "[Error] Invalid ray direction.\n";
        return RayHit{};
    }
    ray_direction.x = ray_direction.x / len;
    ray_direction.y = ray_direction.y / len;

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


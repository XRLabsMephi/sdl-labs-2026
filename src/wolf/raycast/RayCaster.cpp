#include <raycast/RayCaster.h>
#include <algorithm>
#include <core/Color.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <cassert>

using namespace raycast;


void RayCaster::drawByRayHit(const RayHit& ray_hit, std::pair<double, double> cos_vertical_horizontal, double depth, int column) const{
    /////////////////////////////////////////// parameters
    //TODO как то организовать ввод и получение параметров

    //ground parameters
    core::Color GROUND_COLOR = core::Color::lerp(core::Color::black, core::Color::white, 30);

    //sky parameters
    core::Color SKY_COLOR = core::Color::lerp(core::Color::blue, GROUND_COLOR, 30);

    //wall parameters
    const int W_H = frame_height_; // the height of wall in pixels
    core::Color W_COLOR = core::Color::red;

    //other parameters
    constexpr double W_P_rel = 2.0; // the relation between player and wall height
    constexpr double P_W_rel = 1.0 / W_P_rel; // the relation between wall and player height
    const int P_H = static_cast<int>(W_H * P_W_rel); // the height of player in pixels

    constexpr double SC_D = 1.0; //distance between camera and virtual screen

    double minimal_brightness = 0.6;
    //constexpr double vertical_field_of_view = 0.0; //the angle of view in vertical axe. Interchangeably with SC_D

    ///////////////////////////////////////////

    const double SC_RAY_DEPTH_rel = SC_D/depth;

    int y0 = static_cast<int>(P_H * (1 - SC_RAY_DEPTH_rel)); //bottom border of wall projection in pixels(can be negative)
    int y1 = static_cast<int>(P_H + SC_RAY_DEPTH_rel * (W_H - P_H)); //top border of wall projection in pixels(can be more than frame_height_)

    auto get_draw_params = [&](int color_id) -> core::Color{
        core::Color color;
        double brightness = 1.0;
        switch (color_id){
            case 1:{
                color = W_COLOR;
                //calculating for shadows
                double brightness_procent = ray_hit.face_index_ == IntersectionSide::Vertical ?
                                            cos_vertical_horizontal.first : cos_vertical_horizontal.second;
                //for smooth changing
                brightness = (1 - minimal_brightness) * brightness_procent + minimal_brightness;
                break;
                }
            case 0:
                color = GROUND_COLOR; break;
            case 2:
                color = SKY_COLOR; break;
            default: break;
        }

        color.multiplyColorsByScalar(static_cast<Uint8>(std::lround(255.0 * brightness)));
        return color;
    };

    frame_buffer_.vline(column, 0, std::min(y0, P_H), get_draw_params(2));
    frame_buffer_.vline(column, y0, y1, get_draw_params(ray_hit.wall_id_));
    frame_buffer_.vline(column, y1 + 1, frame_height_, get_draw_params(0));
}


void RayCaster::renderScene() const{

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


    //TODO реализовать другие варианты для запуска лучей(в середины отрезков, по равным углам ...)

    // the step vector
    Vector2 shift = {2.0f * plane.x / static_cast<double>(frame_width_ - 1), 2.0f * plane.y / static_cast<double>(frame_width_ - 1)};
    Vector2 ray_direction = camera_direction - plane; // the first direction

    //the cos of the angle between camera direction and corresponding side of cell normal
    std::pair<double, double> cos_vertical_horizontal = {std::abs(camera_direction.x), std::abs(camera_direction.y)};
    //сейчас реализация такова, что делим на w - 1 отрезок и берем границы в качестве векторов направления
    /*возможно она будет изменена или просто будут добавлены дополнительно расчет
     *на основе равных углов или привязки к серединам отрезков*/
    std::vector<double> wallDepths(frame_width_);
    for (int xi = 0; xi < frame_width_; ++xi, ray_direction += shift) //calculate next direction of ray
    {

        // можно оптимизировать препроцессинг и избавиться от корней в теории
        //normalization
        RayHit ray_hit = castRay(ray_direction);

        //projection of ray vector way on camera vector
        double depth = ray_hit.t_distance_ * (ray_direction.x * camera_direction.x + ray_direction.y * camera_direction.y);


        wallDepths[xi] = depth;
        drawByRayHit(ray_hit, cos_vertical_horizontal, depth, xi);
    }

    drawSprites(camera_direction, tan_half_fov, wallDepths);
}

void RayCaster::drawSprites(
    Vector2 forward,
    double tanHalfFov,
    const std::vector<double>& wallDepths
) const {
    struct VisibleSprite {
        const world::Sprite* sprite;
        double cameraX;
        double depth;
    };

    const Vector2 right{forward.y, -forward.x};
    const double halfWidth = (frame_width_ - 1) / 2.0;
    const double horizontalFocalLength = halfWidth / tanHalfFov;
    const double horizon = frame_height_ / 2.0;
    constexpr std::int64_t TEXTURE_UNIT = 1 << 16;

    std::vector<VisibleSprite> visible;
    for (const auto& sprite : map_.getSprites()) {
        const Vector2 relative = sprite.position - player_.getPosition();
        const double depth = relative.dot(forward);
        if (depth > eps) {
            visible.push_back({&sprite, relative.dot(right), depth});
        }
    }

    // Blend nearer sprites over farther sprites and the wall pass.
    std::stable_sort(visible.begin(), visible.end(),
        [](const VisibleSprite& lhs, const VisibleSprite& rhs) {
            return lhs.depth > rhs.depth;
        });

    for (const auto& [sprite, cameraX, depth] : visible) {
        const auto& texture = textures_.getOrEmpty(sprite->textureId);
        const double centerX = halfWidth + horizontalFocalLength * cameraX / depth;
        const double projectedHeight = frame_height_ * (sprite->scale / depth);
        const double projectedWidth = horizontalFocalLength * (sprite->scale / depth)
            * (static_cast<double>(texture.width) / texture.height);

        // Align the base of the sprite with the base of a wall at the same depth.
        const double bottom = horizon + horizon / depth;
        const double top = bottom - projectedHeight;
        const double left = centerX - projectedWidth / 2.0;
        const double rightEdge = centerX + projectedWidth / 2.0;
        if (!std::isfinite(projectedWidth) || !std::isfinite(projectedHeight) ||
            projectedWidth <= 0.0 || projectedHeight <= 0.0) {
            continue;
        }

        // Keep the original bounds for texture coordinates after clipping.
        const auto clip = [](double edge, int limit) {
            return static_cast<int>(std::ceil(std::clamp(edge, 0.0, static_cast<double>(limit))));
        };
        const int x0 = clip(left, frame_width_);
        const int xEnd = clip(rightEdge, frame_width_);
        const int y0 = clip(top, frame_height_);
        const int yEnd = clip(bottom, frame_height_);
        if (x0 >= xEnd || y0 >= yEnd) {
            continue;
        }

        const auto lastTexel = static_cast<std::int64_t>(texture.height) * TEXTURE_UNIT - 1;
        const double textureStep = static_cast<double>(texture.height) * TEXTURE_UNIT / projectedHeight;
        const auto texY = static_cast<std::int64_t>(
            std::clamp((y0 - top) / projectedHeight, 0.0, 1.0) * lastTexel
        );
        const auto step = yEnd - y0 > 1
            ? std::min(static_cast<std::int64_t>(textureStep), (lastTexel - texY) / (yEnd - y0 - 1))
            : 0;

        for (int x = x0; x < xEnd; ++x) {
            if (depth >= wallDepths[x]) {
                continue;
            }
            const int texX = std::min(texture.width - 1,
                static_cast<int>(std::clamp((x - left) / projectedWidth, 0.0, 1.0) * texture.width));
            frame_buffer_.vline_sprite(x, y0, yEnd - 1, {
                texture.column(texX), texY, step, static_cast<float>(depth), 255
            });
        }
    }
}

RayHit RayCaster::castRay(Vector2 ray_direction) const{
    Vector2 ray_coordinate = player_.getPosition();

    auto [ray_dir_x, ray_dir_y] = ray_direction;
    auto [x, y] = ray_coordinate;

    //preprocessing
    //TODO оптимизировать алгоритм, избавить от привязки к единичному вектору направление путем вычислений в его единицах исходной длины

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
    //main loop
    //calculating the next cell and check is there wall
    //TODO более подробно расписать работу алгоритма
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
    double t_distance = t_inter;
    int wall_id = map_.getCell(cell_x, cell_y);

    //previous shadow calculating by thae angle between wall noramll and direction
    //cos of the angle through x-axe
    //double dir_square_cos_x = ray_dir_x * ray_dir_x / (ray_dir_x * ray_dir_x + ray_dir_y * ray_dir_y);
    //cos of the hit angle
    //double hit_cos = sqrt(intersection_side == IntersectionSide::Vertical ? dir_square_cos_x: 1.0f - dir_square_cos_x);

    ray_coordinate.x += ray_dir_x * t_inter;
    ray_coordinate.y += ray_dir_y * t_inter;
    //в данный момент не ясно как обрабатывать и получать rel_wall
    RayHit rayHit(true, t_distance, ray_coordinate, wall_id, intersection_side, 0);
    return rayHit;
}

#include <raycast/RayCaster.h>
#include <render/FrameBuffer.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <algorithm>
#include <render/TextureProjection.h>
#include <utility>

using namespace raycast;
using logging::Error;
using logging::ErrorCode;

void RayCaster::drawByRayHit(const std::pair<double, double>& normal_cos, double depth,
                             const render::Projection& projection, int column) const {
    const int horizon = static_cast<int>(projection.horizon());
    frame_buffer_.vline(column, 0, horizon - 1, view_settings_.sky_color);
    frame_buffer_.vline(column, horizon, frame_height_ - 1, view_settings_.ground_color);
    if (!ray_cast_hit.has_hit_ || depth <= eps || !std::isfinite(depth)) return;

    const double top = projection.screenY(1.0, depth);
    const double bottom = projection.screenY(0.0, depth);
    const auto clip = [this](double y) {
        return static_cast<int>(std::ceil(std::clamp(y, 0.0, static_cast<double>(frame_height_))));
    };
    const int first = clip(top);
    const int end = clip(bottom);
    if (first >= end) return;

    const auto fog = fog_.factor(depth);
    const auto wall = core::Color::fromBits(
        fog_.applyFog(static_cast<Uint32>(view_settings_.wall_color), fog));
    frame_buffer_.vline(column, first, end - 1, wall);
    if (ray_cast_hit.wall_id_ < 2) return;

    const auto& texture = storage_.getOrEmpty(static_cast<size_t>(ray_cast_hit.wall_id_));
    if (&texture == &render::Texture::empty) return;
    if (texture.width <= 0 || texture.height <= 0) return;
    const int tex_x = std::clamp(static_cast<int>(ray_cast_hit.wall_u_ * texture.width),
                                0, texture.width - 1);
    const double cosine = ray_cast_hit.face_index_ == IntersectionSide::Vertical
        ? normal_cos.first : normal_cos.second;
    const double light = std::max(0.5, cosine)
        * std::max(0.0, 1.0 - depth / view_settings_.darkening_distance);
    const auto brightness = static_cast<Uint8>(
        255 * (view_settings_.minimal_brightness + (1.0 - view_settings_.minimal_brightness) * light));
    render::DrawCallParams params{texture.column(tex_x), 0, 0, depth, brightness};
    int y0, y_end;
    if (render::projectTextureColumn(top, bottom, frame_height_, texture.height, y0, y_end, params)) {
        frame_buffer_.vline<false>(column, y0, y_end - 1, params);
    }
}

void RayCaster::renderScene() {
    const Vector2 camera_direction = player_.getDirection();
    const double field_of_view = player_.getAngle();
    if (!std::isfinite(camera_direction.length()) ||
        std::abs(camera_direction.length() - 1.0) > eps ||
        !(field_of_view > angle_eps && field_of_view < pi - angle_eps)) {
        throw Error(ErrorCode::RayCamera, "Invalid camera direction or field of view");
    }
    const render::Projection projection(frame_width_, frame_height_, field_of_view,
                                        player_.getEyeHeight(), view_settings_);
    double tan_half_fov = tan(field_of_view / 2.0f);

    //Further, we turn camera direction by -pi/2 to get orthogonal vector and transform it to necessary for current fov length

    // the vector of half of virtual screen (where image is projected)
    Vector2 plane = Vector2{camera_direction.y, -camera_direction.x} * tan_half_fov;
    // the step vector
    Vector2 shift = plane * (2.0 / static_cast<double>(frame_width_ - 1));

    //show from which position of segment casting start
    constexpr double start_fraction = 0.0;
    Vector2 ray_direction = camera_direction - plane + shift * start_fraction; // the first direction

    //the cos of the angle between camera direction and corresponding side of cell normal
    std::pair<double, double> cos_vertical_horizontal = {std::abs(camera_direction.x), std::abs(camera_direction.y)};

    Vector2 rotate = {cos(field_of_view/ (frame_width_ - 1)), sin(field_of_view/(frame_width_ - 1))};

    auto next_direction = [&rotate, &shift](const Vector2& direction, bool angle_mode = false){
        if (angle_mode){return Vector2{direction.x * rotate.x + direction.y * rotate.y,
                                      -direction.x * rotate.y + direction.y * rotate.x};}
        return direction + shift;
    };

    for (int xi = 0; xi < frame_width_; ++xi, ray_direction = next_direction(ray_direction, view_settings_.angle_projection)){ //calculate next direction of ray
        castRay(ray_direction);
        const double depth = ray_cast_hit.t_distance_ * ray_direction.dot(camera_direction);
        depth_buffer_[xi] = ray_cast_hit.has_hit_ ? depth : std::numeric_limits<double>::infinity();
        drawByRayHit(cos_vertical_horizontal,
                     projection.renderDepth(depth, projection.slopeAt(xi)), projection, xi);
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

    //main loop
    //calculating the next cell and check is there wall
    //TODO более подробно расписать работу алгоритма
    while (!map_.isWall(cell_x, cell_y)){
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
            if (map_.isWall(cell_x + sign_dx, cell_y)){
                cell_x += sign_dx;
                t_inter = closest_t_x;
                intersection_side = IntersectionSide::Vertical;
                break;
            }
            if (map_.isWall(cell_x, cell_y + sign_dy)){
                cell_y += sign_dy;
                t_inter = closest_t_y;
                intersection_side = IntersectionSide::Horizontal;
                break;
            }
            t_inter = closest_t_x;
            intersection_side = IntersectionSide::Vertical;
            cell_x += sign_dx;
            cell_y += sign_dy;
            closest_t_x += tdx;
            closest_t_y += tdy;

        }
    }
    int wall_id = map_.getCell(cell_x, cell_y);
    if (wall_id == -1){
        ray_cast_hit = RayHit{};
        return;
    }
    Vector2 coordinates = ray_coordinate + ray_direction * t_inter;
    ray_cast_hit = RayHit(
        true, t_inter,
        coordinates, wall_id, intersection_side,
        intersection_side == IntersectionSide::Horizontal ?
        coordinates.x - std::floor(coordinates.x): coordinates.y - std::floor(coordinates.y)
    );
}

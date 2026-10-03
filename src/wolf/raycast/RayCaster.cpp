#include "raycast/RayCaster.hpp"
#include "core/Color.hpp"
#include "core/Vector2.hpp"
#include <compare>

RayHit RayCaster::cast_ray(const Player& player, const Map& map, double cameraX) const { //реализация алгоритма DDA
    RayHit result;
    Vector2 rayDir = player.direction + player.camera_plane * cameraX;
    int mapX = floor(player.position.x);
    int mapY = floor(player.position.y);
    double deltaDistX = (std::is_eq(rayDir.x <=> 0.0)) ? 1e30 : std::abs(1.0 / rayDir.x);
    double deltaDistY = (std::is_eq(rayDir.y <=> 0.0)) ? 1e30 : std::abs(1.0 / rayDir.y);
    int stepX=(std::is_eq(rayDir.x <=> 0.0)) ? 1 : static_cast<int>(std::abs(rayDir.x) / rayDir.x);
    int stepY=(std::is_eq(rayDir.y <=> 0.0)) ? 1 : static_cast<int>(std::abs(rayDir.y) / rayDir.y);
    double sideDistX=0.0;
    double sideDistY=0.0;
    if (rayDir.x < 0) {
        sideDistX = (player.position.x - mapX) * deltaDistX;
    } else {
        sideDistX = (mapX + 1.0 - player.position.x) * deltaDistX;
    }
    if (rayDir.y < 0) {
        sideDistY = (player.position.y - mapY) * deltaDistY;
    } else {
        sideDistY = (mapY + 1.0 - player.position.y) * deltaDistY;
    }
    short& side = result.side;
    bool& hit = result.hit;
    while (!hit) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            side=0;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side=1;
        }
        if (map.is_wall(mapX, mapY)) {
            hit = true;
        }
    }
    double perpWallDist = 0.0;
    if (side == 0) {
        perpWallDist = sideDistX - deltaDistX;
    } else {
        perpWallDist = sideDistY - deltaDistY;
    }
    result.depth = perpWallDist;
    result.x = std::max(0, mapX);
    result.y = std::max(0, mapY);
    result.wall_id = map.at(mapX, mapY);
    switch(side) {
        case 0:
            if (stepX < 0) {
                side = 1;
            } else {
                side = 3;
            }
            break;
        case 1:
            if (stepY < 0) {
                side = 0;
            } else {
                side = 2;
            }
    }
    return result;
}

void RayCaster::render(const Player& player, const Map& map, FrameBuffer& buffer) {
    size_t width = buffer.get_width();
    size_t height = buffer.get_height();
    if (depth_buffer_.size() != width) {
        depth_buffer_.resize(width);
    }
    Color ceiling_color = rgb(40, 40, 40);
    Color floor_color = rgb(20, 20, 20);
    for (size_t x = 0; x < width; ++x) {
        double cameraX = 2.0 * x / static_cast<double>(width) - 1.0;
        RayHit hit = cast_ray(player, map, cameraX);
        depth_buffer_[x] = hit.depth;
        int line_height = static_cast<int>(height / hit.depth);
        int draw_start = -line_height / 2 + static_cast<int>(height) / 2;
        int draw_end = line_height / 2 + static_cast<int>(height) / 2;
        buffer.draw_vertical_line(x, 0, draw_start-1, ceiling_color);
        Color wall_color = rgb(0, 180, 220);
        if (hit.side == 0 || hit.side == 2) {
            wall_color = rgb(0, 90, 110);
        } 
        buffer.draw_vertical_line(x, draw_start, draw_end, wall_color);
        buffer.draw_vertical_line(x, draw_end + 1, static_cast<int>(height) - 1, floor_color);
    }
}

const std::vector<double>& RayCaster::get_depth() const {
    return depth_buffer_;
}
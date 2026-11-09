#include "world/Map.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <queue>
#include <algorithm>

void Map::clear() {
    width_ = 0;
    height_ = 0;
    map_.clear();
}

bool Map::load_from_file(const std::string &path) {
    clear();

    std::ifstream file(path);
    if (!file) { return false; }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        height_++;

        std::istringstream iss(line);

        int row_width = 0;

        int x;
        while (iss >> x) {
            map_.push_back(x);
            row_width++;
        }

        if (width_ == 0) { width_ = row_width; }
        else if (row_width != width_) {
            clear();
            return false;
        }
    }

    if (map_.empty()) {
        clear();
        return false;
    }
    return true;
}

bool Map::is_in_map(int x, int y) const {
    return (
            x >= 0 &&
            x < width_ &&
            y >= 0 &&
            y < height_
    );
}

int Map::at(int x, int y) const {
    if (!is_in_map(x, y)) return -1;

    return map_[y * width_ + x];
}

bool Map::is_wall(int x, int y) const {
    if (!is_in_map(x, y)) return true; // за границей карты - стена
    return at(x, y) != 0;
}

bool Map::is_wall(double x, double y) const {
    int x_ = std::floor(x);
    int y_ = std::floor(y);
    return is_wall(x_, y_);
}

std::optional<std::pair<int, int>> Map::find_nearest_free_cell(double x, double y) const {
    if (width_ <= 0 || height_ <= 0) return std::nullopt;

    x = std::clamp((int)std::floor(x), 0, width_  - 1);
    y = std::clamp((int)std::floor(y), 0, height_ - 1);

    std::queue<std::pair<int, int>> queue;
    queue.emplace(x, y);

    std::vector visited(height_, std::vector<int>(width_));

    while (!queue.empty()) {
        auto [cur_x, cur_y] = queue.front();
        queue.pop();

        if (
            cur_x < 0 || cur_x >= width_ ||
            cur_y < 0 || cur_y >= height_ ||
            visited[cur_y][cur_x]
        ) {
            continue;
        }

        visited[cur_y][cur_x] = 1;

        if (!is_wall(cur_x, cur_y)) return std::make_pair(cur_x, cur_y);

        queue.emplace(cur_x + 1,cur_y);
        queue.emplace(cur_x - 1,cur_y);
        queue.emplace(cur_x,cur_y + 1);
        queue.emplace(cur_x,cur_y - 1);
    }

    return std::nullopt;
}

bool Map::circle_hits_wall(Vector2 center, double radius) const {
    if (radius <= 0.0) {
        int cx = static_cast<int>(std::floor(center.x));
        int cy = static_cast<int>(std::floor(center.y));
        return !is_in_map(cx, cy) || is_wall(cx, cy);
    }

    int min_cx = static_cast<int>(std::floor(center.x - radius));
    int max_cx = static_cast<int>(std::floor(center.x + radius));
    int min_cy = static_cast<int>(std::floor(center.y - radius));
    int max_cy = static_cast<int>(std::floor(center.y + radius));

    for (int cy = min_cy; cy <= max_cy; ++cy) {
        for (int cx = min_cx; cx <= max_cx; ++cx) {
            if (!is_in_map(cx, cy)) return true;
            if (!is_wall(cx, cy))   continue;

            double closest_x = std::clamp(center.x, static_cast<double>(cx),
                                          static_cast<double>(cx + 1));
            double closest_y = std::clamp(center.y, static_cast<double>(cy),
                                          static_cast<double>(cy + 1));
            double dx = center.x - closest_x;
            double dy = center.y - closest_y;
            if (dx * dx + dy * dy < radius * radius) return true;
        }
    }
    return false;
}
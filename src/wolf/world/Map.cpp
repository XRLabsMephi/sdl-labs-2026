#include "world/Map.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <queue>
#include <algorithm>
#include <iostream>

void Map::clear() {
    width_ = 0;
    height_ = 0;
    map_.clear();
}

bool Map::load_from_file(const std::string& path) {
    clear();

    std::ifstream file(path);
    if (!file) {
        std::cerr << "[Map] cannot open: " << path << "\n";
        return false;
    }

    constexpr int MIN_CELL_ID = 0;
    constexpr int MAX_CELL_ID = 1;

    std::string line;
    int line_no = 0;

    while (std::getline(file, line)) {
        ++line_no;

        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.empty()) continue;

        std::istringstream iss(line);
        std::vector<int> row;

        int value;
        while (iss >> value) {
            if (value < MIN_CELL_ID || value > MAX_CELL_ID) {
                std::cerr << "[Map] " << path << ":" << line_no
                          << ": invalid wall id " << value
                          << " (expected " << MIN_CELL_ID
                          << ".." << MAX_CELL_ID << ")\n";
                clear();
                return false;
            }
            row.push_back(value);
        }

        iss >> std::ws;
        if (!iss.eof()) {
            std::cerr << "[Map] " << path << ":" << line_no
                      << ": trailing garbage after column " << row.size() << "\n";
            clear();
            return false;
        }

        if (row.empty()) {
            std::cerr << "[Map] " << path << ":" << line_no
                      << ": row contains no cells\n";
            clear();
            return false;
        }

        if (width_ == 0) {
            width_ = static_cast<int>(row.size());
        } else if (static_cast<int>(row.size()) != width_) {
            std::cerr << "[Map] " << path << ":" << line_no
                      << ": row has " << row.size() << " cells, expected " << width_ << "\n";
            clear();
            return false;
        }

        map_.insert(map_.end(), row.begin(), row.end());
        ++height_;
    }

    if (map_.empty() || width_ <= 0 || height_ <= 0) {
        std::cerr << "[Map] " << path << ": empty map\n";
        clear();
        return false;
    }

    if (map_.size() != static_cast<size_t>(width_) * static_cast<size_t>(height_)) {
        std::cerr << "[Map] internal invariant broken: size=" << map_.size()
                  << " expected=" << width_ * height_ << "\n";
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

bool Map::square_hits_wall(Vector2 center, double half_size) const {
    if (half_size <= 0.0) {
        int cx = static_cast<int>(std::floor(center.x));
        int cy = static_cast<int>(std::floor(center.y));
        return !is_in_map(cx, cy) || is_wall(cx, cy);
    }

    double min_x = center.x - half_size;
    double max_x = center.x + half_size;
    double min_y = center.y - half_size;
    double max_y = center.y + half_size;

    int min_cx = static_cast<int>(std::floor(min_x));
    int max_cx = static_cast<int>(std::floor(max_x));
    int min_cy = static_cast<int>(std::floor(min_y));
    int max_cy = static_cast<int>(std::floor(max_y));

    for (int cy = min_cy; cy <= max_cy; ++cy) {
        for (int cx = min_cx; cx <= max_cx; ++cx) {
            if (!is_in_map(cx, cy)) return true;
            if (!is_wall(cx, cy))   continue;

            if (max_x > cx && min_x < cx + 1 &&
                max_y > cy && min_y < cy + 1) {
                return true;
            }
        }
    }
    return false;
}
#include "wolf/world/Map.hpp"

#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>

namespace wolf {

Map::Map(std::vector<std::string> cells) : cells_(std::move(cells)) {
    if (cells_.empty() || cells_.front().empty()
        || cells_.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())
        || cells_.front().size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("Некорректный размер карты");
    }
    for (int y = 0; y < height(); ++y) {
        if (cells_[y].size() != cells_.front().size()) {
            throw std::invalid_argument("Строки карты должны иметь одинаковую длину");
        }
        for (int x = 0; x < width(); ++x) {
            const char value = cells_[y][x];
            if (value < '0' || value > '9') {
                throw std::invalid_argument("Карта должна содержать только цифры от 0 до 9");
            }
            if ((x == 0 || y == 0 || x == width() - 1 || y == height() - 1)
                && value == '0') {
                throw std::invalid_argument("Карта должна быть окружена стенами");
            }
        }
    }
}

Map Map::load(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Не удалось открыть карту: " + path.string());
    }
    std::vector<std::string> rows;
    std::string row;
    while (std::getline(input, row)) {
        if (!row.empty() && row.back() == '\r') {
            row.pop_back();
        }
        rows.push_back(row);
    }
    if (input.bad()) {
        throw std::runtime_error("Ошибка чтения карты: " + path.string());
    }
    return Map(std::move(rows));
}

int Map::width() const { return static_cast<int>(cells_.front().size()); }
int Map::height() const { return static_cast<int>(cells_.size()); }

bool Map::contains(int x, int y) const {
    return x >= 0 && y >= 0 && x < width() && y < height();
}

int Map::cell(int x, int y) const {
    return contains(x, y) ? cells_[y][x] - '0' : 1;
}

bool Map::isWall(int x, int y) const { return cell(x, y) != 0; }

}

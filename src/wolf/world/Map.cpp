#include "wolf/world/Map.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace wolf
{

Map::Map(int width, int height, std::vector<int> cells)
    : width_(width), height_(height), cells_(std::move(cells))
{
    if (width_ <= 0 || height_ <= 0)
    {
        throw std::invalid_argument("Map dimensions must be positive");
    }

    const auto mapWidth = static_cast<std::size_t>(width_);
    const auto mapHeight = static_cast<std::size_t>(height_);
    if (mapWidth > cells_.max_size() / mapHeight ||
        cells_.size() != mapWidth * mapHeight)
    {
        throw std::invalid_argument("Map cell count must match its dimensions");
    }

    if (std::any_of(cells_.begin(), cells_.end(), [](int cell) { return cell < 0; }))
    {
        throw std::invalid_argument("Map cell values must be nonnegative");
    }
}

Map& Map::operator=(const Map& other)
{
    if (this != &other)
    {
        Map copy(other);
        *this = std::move(copy);
    }
    return *this;
}

Map::Map(Map&& other) noexcept
    : width_(std::exchange(other.width_, 0)),
      height_(std::exchange(other.height_, 0)),
      cells_(std::move(other.cells_))
{
}

Map& Map::operator=(Map&& other) noexcept
{
    if (this != &other)
    {
        cells_ = std::move(other.cells_);
        width_ = std::exchange(other.width_, 0);
        height_ = std::exchange(other.height_, 0);
    }
    return *this;
}

int Map::width() const noexcept
{
    return width_;
}

int Map::height() const noexcept
{
    return height_;
}

bool Map::inBounds(int x, int y) const noexcept
{
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}

std::optional<int> Map::cellAt(int x, int y) const noexcept
{
    if (!inBounds(x, y))
    {
        return std::nullopt;
    }

    const auto index = static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
                       static_cast<std::size_t>(x);
    return cells_[index];
}

bool Map::isEmpty(int x, int y) const noexcept
{
    return cellAt(x, y) == 0;
}

bool Map::isWall(int x, int y) const noexcept
{
    const auto cell = cellAt(x, y);
    return cell && *cell > 0;
}

Map makeDefaultMap()
{
    return Map(12, 10, {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1,
        1, 0, 0, 0, 3, 3, 0, 2, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 2, 2, 0, 0, 0, 0, 0, 3, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    });
}

}

#include "wolf/world/Map.hpp"

#include <stdexcept>

namespace wolf
{

Map::Map(int width, int height, std::vector<int> cells)
    : width_(width)
    , height_(height)
    , cells_(std::move(cells))
{
    if (width_ <= 0 || height_ <= 0 || static_cast<int>(cells_.size()) != width_ * height_)
    {
        throw std::invalid_argument("Map dimensions do not match its cells");
    }
}

int Map::width() const
{
    return width_;
}

int Map::height() const
{
    return height_;
}

bool Map::contains(int x, int y) const
{
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

int Map::tileAt(int x, int y) const
{
    return contains(x, y) ? cells_[y * width_ + x] : 1;
}

bool Map::isWall(int x, int y) const
{
    return tileAt(x, y) != 0;
}

} // namespace wolf

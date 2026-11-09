#include "wolf/world/Map.hpp"

#include <stdexcept>
#include <fstream>
#include <utility>

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

Map Map::loadFromFile(const std::string& path)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Cannot open map: " + path);
    }

    int width = 0;
    int height = 0;
    if (!(input >> width >> height) || width <= 0 || height <= 0)
    {
        throw std::runtime_error("Invalid map dimensions: " + path);
    }

    std::vector<int> cells;
    cells.reserve(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    for (int index = 0; index < width * height; ++index)
    {
        int tile = 0;
        if (!(input >> tile) || tile < 0)
        {
            throw std::runtime_error("Invalid map cell data: " + path);
        }
        cells.push_back(tile);
    }
    return {width, height, std::move(cells)};
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

#include "wolf/world/Map.hpp"

#include <fstream>

namespace wolf
{
bool Map::loadFromFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file)
    {
        return false;
    }

    cells_.clear();

    std::string line;

    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (!line.empty())
        {
            cells_.push_back(line);
        }
    }

    if (cells_.empty())
    {
        return false;
    }

    const std::size_t expectedWidth = cells_.front().size();

    if (expectedWidth == 0)
    {
        return false;
    }

    for (const std::string& row : cells_)
    {
        if (row.size() != expectedWidth)
        {
            cells_.clear();
            return false;
        }
    }

    return true;
}

int Map::width() const
{
    return cells_.empty() ? 0 : static_cast<int>(cells_.front().size());
}

int Map::height() const
{
    return static_cast<int>(cells_.size());
}

int Map::cell(int x, int y) const
{
    if (x < 0 || y < 0 || x >= width() || y >= height())
    {
        return 1;
    }

    const char value = cells_[y][x];

    if (value < '0' || value > '9')
    {
        return 1;
    }

    return value - '0';
}

bool Map::isWall(int x, int y) const
{
    return cell(x, y) != 0;
}
}

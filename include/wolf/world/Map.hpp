#pragma once

#include <vector>

namespace wolf
{

class Map
{
public:
    Map(int width, int height, std::vector<int> cells);

    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] bool contains(int x, int y) const;
    [[nodiscard]] int tileAt(int x, int y) const;
    [[nodiscard]] bool isWall(int x, int y) const;

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<int> cells_;
};

} // namespace wolf

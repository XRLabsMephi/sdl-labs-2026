#pragma once

#include <string>
#include <vector>

namespace wolf
{
class Map
{
public:
    bool loadFromFile(const std::string& path);
    int width() const;
    int height() const;
    int cell(int x, int y) const;
    bool isWall(int x, int y) const;

private:
    std::vector<std::string> cells_;
};
}

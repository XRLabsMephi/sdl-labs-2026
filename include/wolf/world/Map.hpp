#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace wolf {

class Map {
public:
    explicit Map(std::vector<std::string> cells);
    static Map load(const std::filesystem::path& path);

    int width() const;
    int height() const;
    bool contains(int x, int y) const;
    int cell(int x, int y) const;
    bool isWall(int x, int y) const;

private:
    std::vector<std::string> cells_;
};

}

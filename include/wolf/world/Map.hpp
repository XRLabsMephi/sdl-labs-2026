#pragma once
#include <vector>

namespace wolf::world {
    class Map {
    public:
        Map();
        int getCell(int x, int y) const;
        int getWidth() const;
        int getHeight() const;

    private:
        int width;
        int height;
        std::vector<int> grid;
    };
}
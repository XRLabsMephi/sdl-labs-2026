#pragma once

#include <string>
#include <vector>

namespace world {
    // Хранит вектор векторов интов: 0 - пусто, 1...9 - стена
    // cellAt(x, y) - id стены или 0.
    // isSolid(x, y) - стена или выход за границу.
    class Map {
    public:
        Map() = default;

        explicit Map(std::vector<std::vector<int>> cells);

        static Map loadFromFile(const std::string &path);

        static Map initMap();

        static Map initMap(const std::string &filePath);

        size_t width() const noexcept;

        size_t height() const noexcept;

        bool isInBounds(int x, int y) const noexcept;

        int cellAt(int x, int y) const noexcept;

        bool isSolid(int x, int y) const noexcept;

    private:
        static bool areMapCellsValid(const std::vector<std::vector<int>> &cells);

        std::vector<std::vector<int>> cells_;
    };
}

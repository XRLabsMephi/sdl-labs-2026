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

        [[nodiscard]] static Map loadFromFile(const std::string &path);

        [[nodiscard]] static Map defaultMap();

        [[nodiscard]] int width() const noexcept;

        [[nodiscard]] int height() const noexcept;

        [[nodiscard]] bool isInBounds(int x, int y) const noexcept;

        [[nodiscard]] int cellAt(int x, int y) const noexcept;

        [[nodiscard]] bool isSolid(int x, int y) const noexcept;

    private:
        std::vector<std::vector<int>> cells_;
    };
}

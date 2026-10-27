#pragma once

#include <optional>
#include <vector>

namespace wolf {

    class Map {
        public:
            Map(int width, int height, std::vector<int> cells);

            Map(const Map&) = default;
            Map& operator=(const Map& other);

            Map(Map&& other) noexcept;
            Map& operator=(Map&& other) noexcept;

            [[nodiscard]] int width() const noexcept;
            [[nodiscard]] int height() const noexcept;
            [[nodiscard]] bool inBounds(int x, int y) const noexcept;

            [[nodiscard]] std::optional<int> cellAt(int x, int y) const noexcept;

            [[nodiscard]] bool isEmpty(int x, int y) const noexcept;
            [[nodiscard]] bool isWall(int x, int y) const noexcept;

        private:
            int width_;
            int height_;
            std::vector<int> cells_;
    };

    [[nodiscard]] Map makeDefaultMap();

}

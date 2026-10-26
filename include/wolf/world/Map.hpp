#pragma once

#include <optional>
#include <vector>

namespace wolf
{

// Cell coordinates start at (0, 0): x grows right, y grows down.
// One cell occupies one world unit along each axis.
class Map
{
public:
    // Cells are stored row by row: cells[y * width + x].
    // 0 is empty; positive values are wall IDs.
    // Throws std::invalid_argument for nonpositive dimensions, a mismatched
    // cell count, or negative cell values.
    Map(int width, int height, std::vector<int> cells);

    Map(const Map&) = default;
    Map& operator=(const Map& other);

    // Moving a map leaves the source with zero width and height.
    Map(Map&& other) noexcept;
    Map& operator=(Map&& other) noexcept;

    [[nodiscard]] int width() const noexcept;
    [[nodiscard]] int height() const noexcept;
    [[nodiscard]] bool inBounds(int x, int y) const noexcept;

    // Returns std::nullopt outside the map; otherwise returns the cell value.
    [[nodiscard]] std::optional<int> cellAt(int x, int y) const noexcept;

    // Both return false outside the map. Movement should require isEmpty().
    [[nodiscard]] bool isEmpty(int x, int y) const noexcept;
    [[nodiscard]] bool isWall(int x, int y) const noexcept;

private:
    int width_;
    int height_;
    std::vector<int> cells_;
};

[[nodiscard]] Map makeDefaultMap();

}

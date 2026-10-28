#include "wolf/world/Map.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void expectInvalidMap(int width, int height, std::vector<int> cells)
{
    try
    {
        const wolf::Map map(width, height, std::move(cells));
    }
    catch (const std::invalid_argument&)
    {
        return;
    }
    throw std::runtime_error("Invalid map data was accepted");
}

void testRectangularMap()
{
    const wolf::Map map(4, 3, {
        0, 1, 2, 0,
        3, 0, 4, 5,
        0, 6, 0, 7,
    });

    require(map.width() == 4 && map.height() == 3, "Incorrect map dimensions");
    require(map.cellAt(0, 1) == 3, "Incorrect row order");
    require(map.cellAt(2, 1) == 4, "Wall ID was not preserved");
    require(map.cellAt(3, 2) == 7, "Last cell is inaccessible");
    require(map.inBounds(0, 0) && map.inBounds(3, 2), "Corners must be in bounds");
    require(map.isEmpty(0, 0) && !map.isWall(0, 0), "Zero must be empty");
    require(map.isWall(3, 2) && !map.isEmpty(3, 2), "Positive IDs must be walls");
}

void testOutsideCoordinates()
{
    const wolf::Map map(2, 2, {0, 1, 2, 0});
    const int min = std::numeric_limits<int>::min();
    const int max = std::numeric_limits<int>::max();
    const std::pair<int, int> outside[] = {
        {-1, 0}, {0, -1}, {2, 0}, {0, 2}, {-1, -1}, {2, 2},
        {min, 0}, {max, 0}, {0, min}, {0, max}, {min, max},
    };

    for (const auto& [x, y] : outside)
    {
        require(!map.inBounds(x, y), "Outside coordinates reported as in bounds");
        require(!map.cellAt(x, y).has_value(), "Outside access must return nullopt");
        require(!map.isEmpty(x, y), "Outside coordinates must not be walkable");
        require(!map.isWall(x, y), "Outside coordinates must not invent a wall ID");
    }
}

void testSmallMapsAndOwnership()
{
    const int max = std::numeric_limits<int>::max();
    const wolf::Map emptyCell(1, 1, {0});
    const wolf::Map wallCell(1, 1, {max});
    const wolf::Map row(3, 1, {0, 2, 3});
    const wolf::Map column(1, 3, {0, 4, 5});
    require(emptyCell.isEmpty(0, 0), "A single empty cell must be valid");
    require(wallCell.isWall(0, 0) && wallCell.cellAt(0, 0) == max,
            "The full positive wall ID range must be preserved");
    require(row.cellAt(2, 0) == 3 && column.cellAt(0, 2) == 5,
            "Single rows and columns must be valid");

    std::vector<int> cells{0, 9};
    const wolf::Map map(2, 1, cells);
    cells[1] = 0;
    require(map.cellAt(1, 0) == 9, "Map must own its cell data");
}

void testInvalidData()
{
    expectInvalidMap(0, 1, {});
    expectInvalidMap(1, 0, {});
    expectInvalidMap(0, 0, {});
    expectInvalidMap(-1, 1, {});
    expectInvalidMap(1, -1, {});
    expectInvalidMap(-1, -1, {0});
    expectInvalidMap(1, 1, {});
    expectInvalidMap(2, 2, {0, 1, 2});
    expectInvalidMap(2, 2, {0, 1, 2, 3, 4});
    expectInvalidMap(2, 2, {0, -1, 2, 3});
    expectInvalidMap(1, 1, {std::numeric_limits<int>::min()});
    expectInvalidMap(std::numeric_limits<int>::max(),
                     std::numeric_limits<int>::max(), {0});
}

void testCopyAndMove()
{
    wolf::Map source(2, 1, {0, 8});
    wolf::Map copy = source;
    wolf::Map moved = std::move(source);
    require(copy.cellAt(1, 0) == 8 && moved.cellAt(1, 0) == 8,
            "Copying or moving must preserve cells");
    require(source.width() == 0 && source.height() == 0 &&
                !source.cellAt(0, 0) && !source.isEmpty(0, 0),
            "A moved-from map must be safely empty");

    wolf::Map assigned(1, 2, {1, 2});
    assigned = copy;
    require(assigned.width() == 2 && assigned.height() == 1 &&
                assigned.cellAt(1, 0) == 8,
            "Copy assignment must replace dimensions and cells together");
    assigned = std::move(moved);
    require(assigned.cellAt(1, 0) == 8 && moved.width() == 0 &&
                moved.height() == 0 && !moved.cellAt(0, 0),
            "Move assignment must leave both maps safe");
    source = copy;
    require(source.cellAt(1, 0) == 8, "A moved-from map must remain assignable");
}

void testDefaultMap()
{
    const auto map = wolf::makeDefaultMap();
    require(map.width() == 12 && map.height() == 10 && map.isEmpty(1, 1),
            "Default map must have a free starting cell");
    for (int x = 0; x < map.width(); ++x)
    {
        require(map.isWall(x, 0) && map.isWall(x, map.height() - 1),
                "Default map must have closed top and bottom borders");
    }
    for (int y = 0; y < map.height(); ++y)
    {
        require(map.isWall(0, y) && map.isWall(map.width() - 1, y),
                "Default map must have closed left and right borders");
    }
    require(map.cellAt(7, 2) == 2 && map.cellAt(4, 4) == 3,
            "Default map must preserve different wall IDs");
}

}

int main()
{
    try
    {
        testRectangularMap();
        testOutsideCoordinates();
        testSmallMapsAndOwnership();
        testInvalidData();
        testCopyAndMove();
        testDefaultMap();
        std::cout << "All Map tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

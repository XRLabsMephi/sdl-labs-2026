#include "wolf/raycast/RayHit.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

    constexpr double defaultPlane = 0.5773502691896257;
    constexpr double infinity = std::numeric_limits<double>::infinity();

    void require(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    bool near(double a, double b, double tolerance = 1e-10) {
        return std::abs(a - b) <= tolerance;
    }

    // Прямоугольная комната, окружённая стенами borderId.
    std::vector<int> roomCells(int width, int height, int borderId = 1) {
        std::vector<int> cells(static_cast<std::size_t>(width * height), 0);
        for (int x = 0; x < width; ++x) {
            cells[x] = borderId;
            cells[(height - 1) * width + x] = borderId;
        }
        for (int y = 0; y < height; ++y) {
            cells[y * width] = borderId;
            cells[y * width + width - 1] = borderId;
        }
        return cells;
    }

    wolf::Map room(int width, int height, int borderId = 1) {
        return wolf::Map(width, height, roomCells(width, height, borderId));
    }

    // Камера под углом angle к оси +X; плоскость камеры направлена вправо от взгляда,
    // как в PlayerController: right = {-direction.y, direction.x}.
    wolf::Player camera(wolf::Vec2 position, double angle, double planeLength = defaultPlane,
                        double directionLength = 1.0) {
        wolf::Player player;
        player.position = position;
        player.direction = {std::cos(angle) * directionLength, std::sin(angle) * directionLength};
        player.cameraPlane = {-std::sin(angle) * planeLength, std::cos(angle) * planeLength};
        return player;
    }

    wolf::Player axisCamera(wolf::Vec2 position, wolf::Vec2 direction) {
        wolf::Player player;
        player.position = position;
        player.direction = direction;
        player.cameraPlane = {-direction.y * defaultPlane, direction.x * defaultPlane};
        return player;
    }

    // Карта 7x5 с разными ID стен по сторонам:
    //   1 1 1 1 1 1 1
    //   4 0 0 0 0 0 2
    //   4 0 0 0 0 0 2
    //   4 0 0 0 0 0 2
    //   1 3 3 3 3 3 1
    wolf::Map labeledRoom() {
        return wolf::Map(7, 5, {
            1, 1, 1, 1, 1, 1, 1,
            4, 0, 0, 0, 0, 0, 2,
            4, 0, 0, 0, 0, 0, 2,
            4, 0, 0, 0, 0, 0, 2,
            1, 3, 3, 3, 3, 3, 1,
        });
    }

    void testCameraRays() {
        require(wolf::cameraXForColumn(0, 960) == -1.0, "The left column must have cameraX = -1");
        require(wolf::cameraXForColumn(480, 960) == 0.0,
                "The central column must have cameraX = 0");
        require(near(wolf::cameraXForColumn(959, 960), 1.0 - 2.0 / 960.0),
                "cameraX must follow 2 * x / width - 1");
        for (int x = 1; x < 960; ++x) {
            require(near(wolf::cameraXForColumn(x, 960), -wolf::cameraXForColumn(960 - x, 960)),
                    "Columns must be symmetric around the screen center");
            require(wolf::cameraXForColumn(x, 960) > wolf::cameraXForColumn(x - 1, 960),
                    "cameraX must grow from left to right");
        }
        require(wolf::cameraXForColumn(0, 0) == 0.0 && wolf::cameraXForColumn(3, -5) == 0.0,
                "An empty screen must not divide by zero");

        const wolf::Player player;
        const wolf::Vec2 center = wolf::rayDirection(player, 0.0);
        const wolf::Vec2 left = wolf::rayDirection(player, -1.0);
        const wolf::Vec2 right = wolf::rayDirection(player, 1.0);
        require(center.x == player.direction.x && center.y == player.direction.y,
                "The central ray must match the view direction");
        require(near(left.x, 1.0) && near(left.y, -defaultPlane) && near(right.x, 1.0) &&
                    near(right.y, defaultPlane),
                "Edge rays must be direction -/+ cameraPlane");
        require(near(std::atan2(right.y, right.x), std::acos(-1.0) / 6.0),
                "The default camera must have a 60 degree field of view");
    }

    void testAxisAlignedHits() {
        const auto map = labeledRoom();
        const wolf::Vec2 position{2.25, 2.5};

        struct Case {
            wolf::Vec2 direction;
            int mapX;
            int mapY;
            int wallId;
            wolf::WallSide side;
            double depth;
            const char* message;
        };
        const Case cases[] = {
            {{1.0, 0.0}, 6, 2, 2, wolf::WallSide::West, 3.75, "+X ray must hit the west face"},
            {{-1.0, 0.0}, 0, 2, 4, wolf::WallSide::East, 1.25, "-X ray must hit the east face"},
            {{0.0, 1.0}, 2, 4, 3, wolf::WallSide::North, 1.5, "+Y ray must hit the north face"},
            {{0.0, -1.0}, 2, 0, 1, wolf::WallSide::South, 1.5, "-Y ray must hit the south face"},
        };
        for (const Case& expected : cases) {
            const auto hit = wolf::castRay(map, axisCamera(position, expected.direction), 0.0);
            require(hit.hit && hit.mapX == expected.mapX && hit.mapY == expected.mapY &&
                        hit.wallId == expected.wallId && hit.side == expected.side &&
                        near(hit.depth, expected.depth),
                    expected.message);
        }
        require(wolf::isXSide(wolf::WallSide::West) && wolf::isXSide(wolf::WallSide::East) &&
                    !wolf::isXSide(wolf::WallSide::North) && !wolf::isXSide(wolf::WallSide::South),
                "West and east faces are X sides");

        const auto onLine = wolf::castRay(map, axisCamera({2.0, 2.5}, {-1.0, 0.0}), 0.0);
        require(onLine.hit && onLine.mapX == 0 && near(onLine.depth, 1.0),
                "A camera on a grid line must step into the next cell immediately");
        const auto touching = wolf::castRay(map, axisCamera({1.0, 2.5}, {-1.0, 0.0}), 0.0);
        require(touching.hit && touching.mapX == 0 && touching.depth == 0.0,
                "A wall touching the camera must have zero depth");

        const auto defaultHit = wolf::castRay(wolf::makeDefaultMap(), wolf::Player{}, 0.0);
        require(defaultHit.hit && defaultHit.mapX == 11 && defaultHit.mapY == 1 &&
                    defaultHit.wallId == 1 && near(defaultHit.depth, 9.5),
                "The default camera must see the east border of the default map");
    }

    void testPerpendicularDepth() {
        const auto map = room(20, 41);
        const auto player = camera({2.5, 20.5}, 0.0);
        double shortestRay = infinity;
        double longestRay = 0.0;
        for (int x = 0; x < 320; ++x) {
            const double cameraX = wolf::cameraXForColumn(x, 320);
            const auto hit = wolf::castRay(map, player, cameraX);
            require(hit.hit && hit.mapX == 19 && hit.side == wolf::WallSide::West,
                    "Every column must hit the flat east wall");
            require(near(hit.depth, 16.5, 1e-9),
                    "A flat wall must have the same depth in every column (no fisheye)");

            const wolf::Vec2 ray = wolf::rayDirection(player, cameraX);
            const double rayLength = hit.depth * std::hypot(ray.x, ray.y);
            shortestRay = std::min(shortestRay, rayLength);
            longestRay = std::max(longestRay, rayLength);
        }
        require(longestRay > shortestRay + 1.0,
                "Edge rays are longer than the depth; using them would curve the wall");

        const auto scaled = camera({2.5, 20.5}, 0.0, 2.0 * defaultPlane, 2.0);
        for (const double cameraX : {-1.0, -0.3, 0.0, 0.7}) {
            require(near(wolf::castRay(map, scaled, cameraX).depth,
                         wolf::castRay(map, player, cameraX).depth, 1e-9),
                    "Depth must be in world units and not depend on the direction length");
        }

        const auto rotated = camera({10.5, 20.5}, std::acos(-1.0) / 2.0);
        const auto down = wolf::castRay(map, rotated, 0.0);
        require(down.hit && down.mapY == 40 && down.side == wolf::WallSide::North &&
                    near(down.depth, 19.5, 1e-9),
                "A rotated camera must measure depth along its own direction");
    }

    void testCorners() {
        const double s = 1.0 / std::sqrt(2.0);
        wolf::Player diagonal;
        diagonal.position = {5.5, 5.5};
        diagonal.direction = {s, s};
        diagonal.cameraPlane = {-s * defaultPlane, s * defaultPlane};

        const auto open = wolf::castRay(room(10, 10), diagonal, 0.0);
        require(open.hit && open.mapX + open.mapY == 17 && near(open.depth, 3.5 * std::sqrt(2.0)),
                "A ray through grid corners must reach the far corner of the room");

        auto cells = roomCells(10, 10);
        cells[5 * 10 + 6] = 5;
        cells[6 * 10 + 5] = 6;
        const wolf::Map crack(10, 10, std::move(cells));
        const auto blocked = wolf::castRay(crack, diagonal, 0.0);
        require(blocked.hit && (blocked.wallId == 5 || blocked.wallId == 6) &&
                    near(blocked.depth, 0.5 * std::sqrt(2.0)),
                "A ray must not leak between two walls touching at a corner");
    }

    struct ExactHit {
        double t = infinity;
        double secondT = infinity;
        double sideMargin = infinity;
        int mapX = 0;
        int mapY = 0;
        int wallId = 0;
        wolf::WallSide side = wolf::WallSide::West;
    };

    // Независимая проверка DDA: точное пересечение луча с каждой клеткой-стеной
    // (метод «плит»), без обхода сетки.
    ExactHit exactFirstWall(const wolf::Map& map, wolf::Vec2 p, wolf::Vec2 d) {
        ExactHit best;
        for (int y = 0; y < map.height(); ++y) {
            for (int x = 0; x < map.width(); ++x) {
                if (!map.isWall(x, y)) {
                    continue;
                }
                const double tx1 = (x - p.x) / d.x;
                const double tx2 = (x + 1 - p.x) / d.x;
                const double ty1 = (y - p.y) / d.y;
                const double ty2 = (y + 1 - p.y) / d.y;
                const double txEnter = std::min(tx1, tx2);
                const double tyEnter = std::min(ty1, ty2);
                const double enter = std::max(txEnter, tyEnter);
                const double exit = std::min(std::max(tx1, tx2), std::max(ty1, ty2));
                if (enter > exit || exit <= 0.0) {
                    continue;
                }
                if (enter < best.t) {
                    best.secondT = best.t;
                    best.t = enter;
                    best.mapX = x;
                    best.mapY = y;
                    best.wallId = *map.cellAt(x, y);
                    best.sideMargin = std::abs(txEnter - tyEnter);
                    best.side = txEnter > tyEnter
                                    ? (d.x > 0.0 ? wolf::WallSide::West : wolf::WallSide::East)
                                    : (d.y > 0.0 ? wolf::WallSide::North : wolf::WallSide::South);
                } else {
                    best.secondT = std::min(best.secondT, enter);
                }
            }
        }
        return best;
    }

    void testAgainstExactIntersection() {
        std::mt19937 random(2026);
        std::uniform_real_distribution<double> unit(0.0, 1.0);
        std::uniform_int_distribution<int> wallId(1, 4);
        const double pi = std::acos(-1.0);

        int checkedCells = 0;
        for (int mapIndex = 0; mapIndex < 6; ++mapIndex) {
            const int width = 24;
            const int height = 18;
            auto cells = roomCells(width, height);
            std::vector<std::pair<int, int>> freeCells;
            for (int y = 1; y < height - 1; ++y) {
                for (int x = 1; x < width - 1; ++x) {
                    if (unit(random) < 0.2) {
                        cells[y * width + x] = wallId(random);
                    } else {
                        freeCells.emplace_back(x, y);
                    }
                }
            }
            const wolf::Map map(width, height, std::move(cells));
            std::uniform_int_distribution<std::size_t> pick(0, freeCells.size() - 1);

            for (int trial = 0; trial < 500; ++trial) {
                const auto [cellX, cellY] = freeCells[pick(random)];
                const auto player = camera({cellX + unit(random), cellY + unit(random)},
                                           2.0 * pi * unit(random), 0.2 + 1.3 * unit(random),
                                           0.5 + 1.5 * unit(random));
                const double cameraX = 2.0 * unit(random) - 1.0;
                const wolf::Vec2 ray = wolf::rayDirection(player, cameraX);

                const auto hit = wolf::castRay(map, player, cameraX);
                const auto exact = exactFirstWall(map, player.position, ray);
                const double forward = (ray.x * player.direction.x + ray.y * player.direction.y) /
                                       std::hypot(player.direction.x, player.direction.y);
                const double expectedDepth = exact.t * forward;
                const double tolerance = 1e-9 * std::max(1.0, expectedDepth);
                require(hit.hit && near(hit.depth, expectedDepth, tolerance),
                        "DDA depth must match the exact first wall intersection");
                if (exact.secondT - exact.t > 1e-9) {
                    require(hit.mapX == exact.mapX && hit.mapY == exact.mapY &&
                                hit.wallId == exact.wallId,
                            "DDA must find the same first wall cell as the exact intersection");
                    ++checkedCells;
                    if (exact.sideMargin > 1e-9) {
                        require(hit.side == exact.side, "DDA must report the face the ray entered");
                    }
                }
            }
        }
        require(checkedCells > 2900, "The exact comparison must cover almost every random ray");
    }

    void testMissesAndInvalidCameras() {
        const wolf::Map open(8, 8, std::vector<int>(64, 0));
        const wolf::Vec2 directions[] = {
            {1.0, 0.0}, {-1.0, 0.0}, {0.0, 1.0}, {0.0, -1.0},
            {1.0, 1.0}, {-1.0, 0.3}, {0.2, -1.0},
        };
        for (const wolf::Vec2 direction : directions) {
            for (const wolf::Vec2 position : {wolf::Vec2{4.0, 4.0}, wolf::Vec2{0.0, 7.9}}) {
                const auto hit = wolf::castRay(open, axisCamera(position, direction), 0.0);
                require(!hit.hit && hit.depth == infinity,
                        "A ray leaving the map without walls must report a miss");
            }
        }

        const auto map = labeledRoom();
        const double nan = std::numeric_limits<double>::quiet_NaN();
        const wolf::Vec2 badPositions[] = {
            {-0.5, 2.5}, {7.0, 2.5}, {2.5, 5.0},
            {1e100, 2.5}, {-1e100, -1e100}, {nan, 2.5}, {infinity, 2.5},
            {0.5, 0.5}, {6.5, 2.5},
        };
        for (const wolf::Vec2 position : badPositions) {
            const auto hit = wolf::castRay(map, axisCamera(position, {1.0, 0.0}), 0.0);
            require(!hit.hit && hit.depth == infinity,
                    "A camera outside the map or inside a wall must not produce a hit");
        }

        wolf::Player player = axisCamera({2.5, 2.5}, {1.0, 0.0});
        player.direction = {};
        require(!wolf::castRay(map, player, 0.0).hit, "A zero direction must not produce a hit");
        player = axisCamera({2.5, 2.5}, {1.0, 0.0});
        player.direction.y = nan;
        require(!wolf::castRay(map, player, 0.0).hit,
                "A nonfinite direction must not produce a hit");
        player = axisCamera({2.5, 2.5}, {1.0, 0.0});
        player.cameraPlane.x = infinity;
        require(!wolf::castRay(map, player, 0.5).hit, "A nonfinite ray must not produce a hit");
        player = axisCamera({2.5, 2.5}, {1.0, 0.0});
        player.cameraPlane = {-1.0, 0.0};
        require(!wolf::castRay(map, player, 1.0).hit,
                "A ray that does not go forward cannot be projected");
    }

    void testProjection() {
        const wolf::Player player;
        require(near(wolf::focalLength(player, 960), 480.0 / std::tan(std::acos(-1.0) / 6.0)),
                "Focal length must match the horizontal field of view");
        require(near(wolf::focalLength(camera({}, 1.0, 1.0, 2.0), 100), 100.0),
                "Focal length must depend on the plane to direction ratio");
        wolf::Player flat;
        flat.cameraPlane = {};
        require(wolf::focalLength(player, 0) == 0.0 && wolf::focalLength(flat, 960) == 0.0,
                "A degenerate camera must have no focal length");

        wolf::RayHit hit;
        hit.hit = true;
        hit.depth = 2.0;
        const auto near2 = wolf::projectWall(hit, 400.0, 600);
        require(near(near2.height, 200.0) && near2.drawStart == 200 && near2.drawEnd == 400,
                "A wall must be centered on the horizon with height focal / depth");
        hit.depth = 4.0;
        const auto far4 = wolf::projectWall(hit, 400.0, 600);
        require(near(far4.height, 100.0) && far4.drawStart == 250 && far4.drawEnd == 350,
                "Doubling the depth must halve the wall height");
        require(far4.hit.depth == 4.0, "The slice must keep its ray hit");

        hit.depth = 3.0;
        const auto odd = wolf::projectWall(hit, 9.0, 5);
        require(odd.drawStart == 1 && odd.drawEnd == 4, "Rows must be chosen by their centers");
        hit.depth = 2.0;
        const auto fractional = wolf::projectWall(hit, 401.0, 600);
        require(fractional.drawStart == 200 && fractional.drawEnd == 400,
                "A partially covered row belongs to the wall only if its center is covered");

        for (const double depth : {0.5, 1e-3, 1e-300, 0.0}) {
            hit.depth = depth;
            const auto close = wolf::projectWall(hit, 831.0, 600);
            require(close.drawStart == 0 && close.drawEnd == 600 && close.height >= 600.0,
                    "A close wall must be clipped to the frame without overflow");
        }

        hit.depth = infinity;
        const auto distant = wolf::projectWall(hit, 400.0, 600);
        require(distant.drawStart == distant.drawEnd, "An infinitely far wall must be empty");

        const wolf::RayHit miss;
        hit.depth = 2.0;
        const double nan = std::numeric_limits<double>::quiet_NaN();
        for (const auto& slice : {wolf::projectWall(miss, 400.0, 600),
                                  wolf::projectWall(hit, 400.0, 0),
                                  wolf::projectWall(hit, 0.0, 600),
                                  wolf::projectWall(hit, nan, 600),
                                  wolf::projectWall(hit, infinity, 600)}) {
            require(slice.drawStart == slice.drawEnd && slice.height == 0.0,
                    "Misses and invalid projections must produce an empty slice");
        }
    }

    void testFrame() {
        const int width = 64;
        const int height = 40;
        auto cells = roomCells(20, 41);
        cells[18 * 20 + 6] = 7;
        const wolf::Map map(20, 41, std::move(cells));
        const auto player = camera({2.5, 20.5}, 0.0);

        wolf::Raycaster raycaster;
        raycaster.castFrame(map, player, width, height);
        const auto& columns = raycaster.columns();
        const auto& depth = raycaster.depthBuffer();
        require(columns.size() == static_cast<std::size_t>(width) &&
                    depth.size() == static_cast<std::size_t>(width),
                "The frame must contain one ray per column");

        const double focal = wolf::focalLength(player, width);
        bool leftSeesMarker = false;
        for (int x = 0; x < width; ++x) {
            const auto expected = wolf::castRay(map, player, wolf::cameraXForColumn(x, width));
            const auto& slice = columns[static_cast<std::size_t>(x)];
            const auto projected = wolf::projectWall(expected, focal, height);
            require(slice.hit.mapX == expected.mapX && slice.hit.mapY == expected.mapY &&
                        slice.hit.depth == expected.depth &&
                        slice.drawStart == projected.drawStart &&
                        slice.drawEnd == projected.drawEnd,
                    "Each column must use its own ray and projection");
            require(depth[static_cast<std::size_t>(x)] == slice.hit.depth,
                    "The depth buffer must store the wall depth of every column");
            require(slice.hit.wallId != 7 || x < width / 2,
                    "A wall to the left of the view must appear on the left half");
            leftSeesMarker = leftSeesMarker || slice.hit.wallId == 7;
        }
        require(leftSeesMarker, "A wall to the left of the view must be visible");

        raycaster.castFrame(room(20, 41), player, width, height);
        for (int x = 1; x < width; ++x) {
            require(near(depth[static_cast<std::size_t>(x)],
                         depth[static_cast<std::size_t>(width - x)], 1e-9),
                    "A symmetric room must produce a symmetric depth buffer");
        }

        raycaster.castFrame(map, player, 0, height);
        require(raycaster.columns().empty() && raycaster.depthBuffer().empty(),
                "An empty frame must clear the previous result");
        raycaster.castFrame(map, player, width, -1);
        require(raycaster.columns().empty() && raycaster.depthBuffer().empty(),
                "A negative frame height must not produce columns");
    }

}

int main() {
    try {
        testCameraRays();
        testAxisAlignedHits();
        testPerpendicularDepth();
        testCorners();
        testAgainstExactIntersection();
        testMissesAndInvalidCameras();
        testProjection();
        testFrame();
        std::cout << "All Raycaster tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

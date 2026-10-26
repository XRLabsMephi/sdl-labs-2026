#include "wolf/input/InputState.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <cmath>
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

bool near(double a, double b, double tolerance = 1e-10)
{
    return std::abs(a - b) <= tolerance;
}

wolf::Map openMap()
{
    return wolf::Map(20, 20, std::vector<int>(400, 0));
}

wolf::Map dividedMap()
{
    std::vector<int> cells(100, 0);
    for (int y = 0; y < 10; ++y)
    {
        cells[y * 10 + 3] = 2;
    }
    return wolf::Map(10, 10, std::move(cells));
}

void testMovementAndTime()
{
    const auto map = openMap();
    const wolf::PlayerController controller;
    wolf::Player player;
    player.position = {5.0, 5.0};
    wolf::InputState input;
    input.moveForward = true;
    controller.update(player, map, input, 0.2);
    require(near(player.position.x, 5.6) && near(player.position.y, 5.0),
            "Forward speed must be 3 world units per second");

    wolf::Player smallFrames;
    smallFrames.position = {5.0, 5.0};
    for (int i = 0; i < 20; ++i)
    {
        controller.update(smallFrames, map, input, 0.01);
    }
    require(near(smallFrames.position.x, player.position.x),
            "Movement must be independent of frame rate");

    input = {};
    input.moveBackward = true;
    controller.update(player, map, input, 0.2);
    require(near(player.position.x, 5.0), "Backward movement must reverse forward movement");
    input = {};
    input.strafeRight = true;
    controller.update(player, map, input, 0.2);
    require(near(player.position.x, 5.0) && near(player.position.y, 5.6),
            "Right strafe must follow the camera's right vector");
    input.strafeRight = false;
    input.strafeLeft = true;
    controller.update(player, map, input, 0.2);
    require(near(player.position.y, 5.0), "Left strafe must reverse right strafe");

    input = {};
    input.moveForward = true;
    input.strafeRight = true;
    controller.update(player, map, input, 0.2);
    require(near(std::hypot(player.position.x - 5.0, player.position.y - 5.0), 0.6),
            "Diagonal movement must not increase speed");
    const auto before = player;
    input.moveBackward = true;
    input.strafeLeft = true;
    input.turnLeft = input.turnRight = true;
    controller.update(player, map, input, 0.2);
    require(near(player.position.x, before.position.x) &&
                near(player.position.y, before.position.y) &&
                near(player.direction.x, before.direction.x),
            "Opposite commands must cancel");
}

void testCameraRotation()
{
    const auto map = openMap();
    const wolf::PlayerController controller;
    wolf::Player player;
    wolf::InputState input;
    input.turnRight = true;
    controller.update(player, map, input, 0.5);
    require(near(player.direction.x, std::cos(0.9)) &&
                near(player.direction.y, std::sin(0.9)),
            "Right rotation must use radians per second and the shared axis convention");
    require(near(player.cameraPlane.x, -0.5773502691896257 * std::sin(0.9)) &&
                near(player.cameraPlane.y, 0.5773502691896257 * std::cos(0.9)),
            "Direction and camera plane must rotate together");
    input.turnRight = false;
    input.turnLeft = true;
    controller.update(player, map, input, 0.5);
    require(near(player.direction.x, 1.0) && near(player.direction.y, 0.0),
            "Left turn must reverse right turn");

    wolf::Player smallFrames;
    input.turnLeft = false;
    input.turnRight = true;
    for (int i = 0; i < 50; ++i)
    {
        controller.update(smallFrames, map, input, 0.01);
    }
    require(near(smallFrames.direction.x, std::cos(0.9)) &&
                near(smallFrames.direction.y, std::sin(0.9)),
            "Rotation must be independent of frame rate");

    for (int i = 0; i < 10000; ++i)
    {
        controller.update(player, map, input, 0.001);
    }
    require(near(std::hypot(player.direction.x, player.direction.y), 1.0) &&
                near(std::hypot(player.cameraPlane.x, player.cameraPlane.y),
                     0.5773502691896257) &&
                near(player.direction.x * player.cameraPlane.x +
                     player.direction.y * player.cameraPlane.y, 0.0),
            "Rotation must preserve camera lengths, perpendicularity, and FOV");
    require(near(player.position.x, 1.5) && near(player.position.y, 1.5),
            "Rotation must not move the player");

    player = {};
    controller.update(player, map, input, 0.5);
    input = {};
    input.moveForward = true;
    controller.update(player, map, input, 0.1);
    require(near(player.position.x, 1.5 + 0.3 * std::cos(0.9)) &&
                near(player.position.y, 1.5 + 0.3 * std::sin(0.9)),
            "Movement must follow the rotated direction");
}

void testCollisionsAndSliding()
{
    const auto map = dividedMap();
    const wolf::PlayerController controller;
    wolf::InputState input;
    input.moveForward = true;
    wolf::Player player;
    player.position = {1.5, 2.5};
    controller.update(player, map, input, 1.0);
    require(player.position.x > 2.5 && player.position.x <= 2.8 + 1e-10,
            "Substeps must prevent crossing a one-cell wall");
    require(map.isEmpty(static_cast<int>(player.position.x),
                        static_cast<int>(player.position.y)),
            "Collision must leave the player in empty space");
    wolf::Player paused;
    paused.position = {1.5, 2.5};
    controller.update(paused, map, input, 1e100);
    require(near(paused.position.x, player.position.x),
            "Large dt must be bounded and must not tunnel through walls");

    player.position = {2.7, 2.5};
    input.strafeRight = true;
    for (int i = 0; i < 10; ++i)
    {
        controller.update(player, map, input, 0.1);
    }
    require(player.position.x <= 2.8 + 1e-10 && player.position.y > 4.5,
            "A blocked X component must still allow sliding along Y");

    auto cornerCells = std::vector<int>(100, 0);
    cornerCells[3 * 10 + 3] = 1;
    const wolf::Map corner(10, 10, std::move(cornerCells));
    player = {};
    player.position = {2.5, 2.5};
    controller.update(player, corner, input, 0.4);
    require(player.position.x + 0.2 < 3.0 || player.position.y + 0.2 < 3.0,
            "The player's footprint must not enter a wall corner");
}

void testBoundariesAndInvalidInput()
{
    const auto map = openMap();
    const wolf::PlayerController controller;
    wolf::Player player;
    player.position = {0.5, 0.5};
    wolf::InputState input;
    input.moveBackward = true;
    input.strafeLeft = true;
    controller.update(player, map, input, 1.0);
    require(player.position.x >= 0.2 && player.position.y >= 0.2,
            "Open map edges must block the entire player footprint");
    player.position = {19.5, 19.5};
    input = {};
    input.moveForward = input.strafeRight = true;
    controller.update(player, map, input, 1.0);
    require(player.position.x + 0.2 < 20.0 && player.position.y + 0.2 < 20.0,
            "Positive map boundaries must block movement");

    const double invalidTimes[] = {0.0, -1.0,
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity()};
    player = {};
    input.turnRight = true;
    for (double dt : invalidTimes)
    {
        controller.update(player, map, input, dt);
        require(near(player.position.x, 1.5) && near(player.position.y, 1.5) &&
                    near(player.direction.x, 1.0) && near(player.cameraPlane.x, 0.0),
                "Invalid dt must not change player state");
    }
    input.quit = true;
    controller.update(player, map, input, 0.1);
    require(near(player.position.x, 1.5), "Quit must not advance the player");
    input.quit = false;
    player.position.x = 1e100;
    controller.update(player, map, input, 0.1);
    require(player.position.x == 1e100, "Invalid positions must not cause integer overflow");
    player = {};
    player.direction = {};
    controller.update(player, map, input, 0.1);
    require(near(player.position.x, 1.5), "A zero direction must not cause division by zero");

    const double invalidComponents[] = {std::numeric_limits<double>::quiet_NaN(),
                                        std::numeric_limits<double>::infinity()};
    for (double value : invalidComponents)
    {
        player = {};
        player.direction.x = value;
        controller.update(player, map, input, 0.1);
        require(near(player.position.x, 1.5) && near(player.cameraPlane.x, 0.0),
                "A nonfinite direction must not change position or camera plane");
        player = {};
        player.cameraPlane.y = value;
        controller.update(player, map, input, 0.1);
        require(near(player.position.x, 1.5) && near(player.direction.x, 1.0),
                "A nonfinite camera plane must not change position or direction");
    }
}

}

int main()
{
    try
    {
        testMovementAndTime();
        testCameraRotation();
        testCollisionsAndSliding();
        testBoundariesAndInvalidInput();
        std::cout << "All PlayerController tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

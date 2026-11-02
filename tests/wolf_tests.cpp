#include "wolf/core/Color.hpp"
#include "wolf/input/InputState.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace {

using namespace wolf;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(double a, double b) { return std::abs(a - b) < 1e-9; }

void testFrameBuffer() {
    FrameBuffer frame(3, 4);
    frame.clear(rgb(10, 20, 30));
    frame.setPixel(-1, 0, 0);
    frame.setPixel(3, 0, 0);
    frame.verticalLine(1, -10, 10, rgb(100, 50, 20));
    for (int y = 0; y < 4; ++y) {
        check(frame.data()[y * 3] == rgb(10, 20, 30), "Границы буфера кадра");
        check(frame.data()[y * 3 + 1] == rgb(100, 50, 20), "Обрезка полосы стены");
    }
    frame.fillRows(2, 10, rgb(1, 2, 3));
    check(frame.data()[6] == rgb(1, 2, 3) && frame.data()[11] == rgb(1, 2, 3),
          "Заливка строк кадра");
}

void testMovement(const Map& map) {
    const PlayerController controller;
    InputState input;
    input.forward = true;
    Player player, many;
    controller.update(player, map, input, 0.1);
    for (int i = 0; i < 10; ++i) controller.update(many, map, input, 0.01);
    check(near(player.position.x, 2.8) && near(player.position.x, many.position.x),
          "Скорость движения не зависит от частоты кадров");
    controller.update(player, map, input, 2.0);
    check(player.position.x <= 5.8 + 1e-9 && controller.canOccupy(map, player.position),
          "Проход сквозь стену");
    player.position = {5.75, 2.5};
    input.strafeRight = true;
    controller.update(player, map, input, 0.1);
    check(player.position.x < 5.8 && player.position.y > 2.5, "Скольжение вдоль стены");
}

void testRaycaster(const Map& map) {
    Raycaster raycaster;
    const auto east = raycaster.castRay(map, {2.5, 2.5}, {1.0, 0.0});
    const auto west = raycaster.castRay(map, {2.5, 2.5}, {-1.0, 0.0});
    const auto south = raycaster.castRay(map, {2.5, 2.5}, {0.0, 1.0});
    check(east.hit && near(east.depth, 3.5) && east.side == WallSide::X,
          "Горизонтальный луч DDA");
    check(west.hit && near(west.depth, 1.5), "Обратный луч DDA");
    check(south.hit && near(south.depth, 5.5) && south.side == WallSide::Y,
          "Вертикальный луч DDA");
    Player camera;
    camera.position = {3.5, 4.5};
    FrameBuffer frame(80, 50);
    raycaster.render(map, camera, frame);
    check(raycaster.depthBuffer().size() == 80, "Размер буфера глубины");
    for (double depth : raycaster.depthBuffer()) check(near(depth, 2.5), "Рыбий глаз");
    check(frame.data()[15 * 80 + 40] == rgb(190, 65, 55)
          && frame.data()[35 * 80 + 40] == rgb(76, 70, 60), "Перспективная высота стены");
}

}

int main() {
    try {
        const wolf::Map map({"1111111", "1000001", "1000001", "1000001", "1000001",
                             "1000001", "1000001", "1000001", "1111111"});
        testFrameBuffer();
        testMovement(map);
        testRaycaster(map);
        std::cout << "Все проверки пройдены\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#include "wolf/core/Color.hpp"
#include "wolf/effects/Fog.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

using namespace wolf;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void testTextures() {
    const auto red = rgb(200, 0, 0), green = rgb(0, 200, 0);
    const auto blue = rgb(0, 0, 200), yellow = rgb(200, 200, 0);
    TextureStore textures;
    textures.set(1, {2, 2, {red, green, blue, yellow}});
    check(textures.get(1).sample(-1, 10) == blue, "Границы текстуры");
    const Map map({"1111111", "1000001", "1000001", "1000001", "1000001",
                   "1000001", "1000001", "1000001", "1111111"});
    Player player;
    player.position = {3.5, 4.25};
    Raycaster raycaster;
    const auto hit = raycaster.castRay(map, player.position, player.direction);
    check(hit.hit && std::abs(hit.wallU - 0.25) < 1e-9, "Координата попадания на стене");
    FrameBuffer frame(80, 50);
    raycaster.render(map, player, frame, &textures);
    check(frame.data()[16 * 80 + 40] == green && frame.data()[34 * 80 + 40] == yellow,
          "Координаты и ориентация текстуры стены");
    Texture stripes{1, 8, {}};
    for (int i = 0; i < 8; ++i) stripes.pixels.push_back(rgb(i * 20, 0, 0));
    textures.set(1, stripes);
    player.position.x = 5.75;
    raycaster.render(map, player, frame, &textures);
    check(frame.data()[40] == rgb(60, 0, 0) && frame.data()[49 * 80 + 40] == rgb(80, 0, 0),
          "Обрезка близкой текстурированной стены");
}

void testSprites() {
    constexpr auto background = rgb(10, 20, 30), red = rgb(200, 0, 0), blue = rgb(0, 0, 200);
    TextureStore textures;
    textures.set(2, {2, 1, {0, red}});
    textures.set(3, {1, 1, {blue}});
    textures.set(4, {1, 1, {red}});
    const Player player;
    const SpriteRenderer renderer;
    FrameBuffer frame(80, 60);
    std::vector<double> depth(80, std::numeric_limits<double>::infinity());
    const std::array near{Sprite{{4.5, 2.5}, 2, 1.0}};
    frame.clear(background);
    renderer.render(near, player, textures, depth, frame);
    check(frame.data()[30 * 80 + 25] == background && frame.data()[30 * 80 + 55] == red,
          "Прозрачный фон предмета");
    const std::array overlap{near[0], Sprite{{5.5, 2.5}, 3, 1.0}};
    frame.clear(background);
    renderer.render(overlap, player, textures, depth, frame);
    check(frame.data()[30 * 80 + 35] == blue && frame.data()[30 * 80 + 45] == red,
          "Сортировка предметов и прозрачные участки");
    const std::array solid{Sprite{{4.5, 2.5}, 4, 1.0}};
    std::fill(depth.begin(), depth.begin() + 40, 1.0);
    frame.clear(background);
    renderer.render(solid, player, textures, depth, frame);
    check(frame.data()[30 * 80 + 35] == background && frame.data()[30 * 80 + 45] == red,
          "Частичное перекрытие предмета стеной");
    const std::array behind{Sprite{{0.5, 2.5}, 3, 1.0}};
    frame.clear(background);
    renderer.render(behind, player, textures, depth, frame);
    check(std::all_of(frame.data(), frame.data() + 80 * 60,
                     [=](auto color) { return color == background; }), "Предмет за камерой");
    check(blend(0x80ff0000u, rgb(0, 0, 255)) == rgb(128, 0, 127), "Полупрозрачность");
}

void testFog() {
    const Fog fog{0.0, 5.0, rgb(0, 0, 0)};
    const auto red = rgb(200, 0, 0);
    check(fog.apply(red, 0.0) == red && fog.apply(red, 5.0) == rgb(0, 0, 0), "Границы тумана");
    check(fog.apply(0x80c80000u, 2.5) == 0x80640000u, "Туман сохраняет прозрачность");
    TextureStore textures;
    textures.set(1, {1, 1, {red}});
    const Map map({"1111111", "1000001", "1000001", "1000001", "1000001",
                   "1000001", "1000001", "1000001", "1111111"});
    Player player;
    player.position = {3.5, 4.5};
    FrameBuffer frame(80, 60);
    Raycaster raycaster;
    raycaster.render(map, player, frame, &textures, &fog);
    for (int x = 0; x < 80; ++x) check(frame.data()[30 * 80 + x] == rgb(100, 0, 0),
                                      "Глубина тумана на плоской стене");
    frame.clear(rgb(0, 0, 0));
    const std::array sprites{Sprite{{6.0, 4.8}, 1, 1.0}};
    std::vector<double> depth(80, std::numeric_limits<double>::infinity());
    SpriteRenderer{}.render(sprites, player, textures, depth, frame, &fog);
    check(frame.data()[30 * 80 + 47] == rgb(100, 0, 0), "Единая глубина стен и предметов");
}

}

int main() {
    try {
        testTextures();
        testSprites();
        testFog();
        std::cout << "Проверки ЛР-2 пройдены\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

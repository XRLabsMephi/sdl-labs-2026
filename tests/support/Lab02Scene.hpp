#pragma once

#include <filesystem>
#include <numbers>
#include <utility>
#include <vector>

#include "wolf/sprite/Sprite.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace lab02 {
    // Participant 1's scene/resource setup is mocked here, outside Application.
    struct DemoScene {
        texture::TextureStore textures;
        world::Map map;
        world::Player player{{2.5, 7.5}, {1.0, 0.0}, 80.0 * std::numbers::pi / 180.0};
        std::vector<sprite::Sprite> sprites{
            {{4.8, 8.7}, 101},              // In front of a wall.
            {{6.0, 7.6}, 102, 0.65},
            {{8.0, 5.8}, 101},              // Partly seen through a glass partition.
            {{10.0, 6.6}, 102, 0.65},       // Seen through glass, in front of an opaque panel.
            {{10.5, 8.9}, 103},
            {{14.0, 7.3}, 101}};            // Almost completely lost in fog.

        DemoScene() {
            const std::filesystem::path root = LAB02_FIXTURES_DIR;
            const auto walls = std::filesystem::path(WOLF_ASSETS_DIR) / "textures/backrooms";
            textures.load(1, walls / "wallpaper.png");
            textures.load(2, walls / "panels.png");
            textures.load(3, walls / "glass.png", 96);
            textures.load(101, root / "barrel.png");
            textures.load(102, root / "medkit.png");
            textures.load(103, root / "lamp.png");

            std::vector<std::vector<int>> cells(15, std::vector<int>(21, 0));
            for (int y = 0; y < 15; ++y) {
                for (int x = 0; x < 21; ++x) {
                    if (y == 0 || y == 14 || x == 0 || x == 20) cells[y][x] = 1;
                }
            }
            for (int y = 6; y < 9; ++y) cells[y][7] = 3;
            cells[7][9] = cells[8][9] = 3;
            for (int y = 4; y < 7; ++y) cells[y][11] = 2;
            cells[9][14] = cells[10][14] = 1;
            map = world::Map(std::move(cells));
        }
    };
}

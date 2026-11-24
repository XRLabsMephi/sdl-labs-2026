#include "wolf/world/SceneSetup.hpp"

#include <cmath>
#include <stdexcept>
#include <string>

namespace wolf {

    void load_scene_textures(TextureStore& textures) {
        for (int id = 1; id <= 5; ++id) {
            textures.load(
                id,
                "assets/wolf/textures/wall0" +
                    std::to_string(id) + ".bmp"
            );
        }

        textures.load(101, "assets/wolf/sprites/barrel.bmp");
        textures.load(102, "assets/wolf/sprites/lamp.bmp");
        textures.load(103, "assets/wolf/sprites/medkit.bmp");
    }

    std::vector<Sprite> create_scene_sprites(const Map& map) {
        std::vector<Sprite> sprites{
            {{3.5, 1.5}, 101, 1.0},
            {{2.5, 2.5}, 102, 1.0},
            {{5.5, 5.5}, 103, 1.0}
        };

        for (const Sprite& sprite : sprites) {
            int x = static_cast<int>(
                std::floor(sprite.position.x)
            );

            int y = static_cast<int>(
                std::floor(sprite.position.y)
            );

            if (!map.in_bounds(x, y) || map.is_wall(x, y)) {
                throw std::runtime_error(
                    "Invalid sprite position for texture: " +
                    std::to_string(sprite.textureId)
                );
            }
        }

        return sprites;
    }

}

#pragma once

#include <vector>

#include "wolf/sprite/Sprite.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"

namespace wolf {

    void load_scene_textures(TextureStore& textures);

    std::vector<Sprite> create_scene_sprites(const Map& map);

}

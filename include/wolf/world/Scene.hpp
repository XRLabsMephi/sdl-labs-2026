#pragma once

#include "wolf/sprite/Sprite.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <filesystem>
#include <vector>

namespace wolf {

struct Scene {
    Map map = makeDefaultMap();
    Player player;
    TextureStore textures;
    std::vector<Sprite> sprites;
};

[[nodiscard]] Scene loadDefaultScene(const std::filesystem::path& resourceDirectory);

} // namespace wolf

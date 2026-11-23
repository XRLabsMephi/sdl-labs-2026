#include "wolf/world/Scene.hpp"

#include "wolf/world/Map.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>

namespace wolf
{

Scene Scene::loadFromFile(const std::string& path, const Map& map)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Cannot open scene: " + path);
    }

    Scene scene;
    while (!(input >> std::ws).eof())
    {
        Sprite sprite;
        if (!(input >> sprite.position.x >> sprite.position.y >> sprite.textureId >> sprite.scale))
        {
            throw std::runtime_error("Malformed sprite data in scene: " + path);
        }
        if (!std::isfinite(sprite.position.x) || !std::isfinite(sprite.position.y)
            || !std::isfinite(sprite.scale) || sprite.scale <= 0.0 || sprite.textureId <= 0
            || sprite.position.x < 0.0 || sprite.position.x >= map.width()
            || sprite.position.y < 0.0 || sprite.position.y >= map.height()
            || map.isWall(static_cast<int>(std::floor(sprite.position.x)),
                          static_cast<int>(std::floor(sprite.position.y))))
        {
            throw std::runtime_error("Invalid sprite in scene: " + path);
        }
        scene.sprites_.push_back(sprite);
    }
    return scene;
}

const std::vector<Sprite>& Scene::sprites() const noexcept
{
    return sprites_;
}

} // namespace wolf

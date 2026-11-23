#pragma once

#include "wolf/sprite/Sprite.hpp"

#include <string>
#include <vector>

namespace wolf
{

class Map;

class Scene
{
public:
    [[nodiscard]] static Scene loadFromFile(const std::string& path, const Map& map);

    [[nodiscard]] const std::vector<Sprite>& sprites() const noexcept;

private:
    std::vector<Sprite> sprites_;
};

} // namespace wolf

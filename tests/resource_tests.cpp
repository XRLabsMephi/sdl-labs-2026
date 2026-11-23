#include "wolf/texture/Texture.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/texture/DemoTextures.hpp"
#include "wolf/texture/ResourceIds.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Scene.hpp"

#include <stdexcept>
#include <string_view>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void requireReadError(auto read, std::string_view expected)
{
    try
    {
        read();
    }
    catch (const std::runtime_error& error)
    {
        if (!std::string_view(error.what()).starts_with(expected))
        {
            throw;
        }
        return;
    }
    throw std::runtime_error("Malformed fixture was accepted");
}

} // namespace

int main()
{
    wolf::Texture texture{2, 1, {0xFF112233U, 0xFF445566U}};
    require(texture.valid(), "Texture dimensions should match its pixels");
    require(texture.sample(1, 0) == 0xFF445566U, "Texture sampling returned the wrong pixel");

    wolf::TextureStore store;
    store.insert(7, std::move(texture));
    require(store.contains(7), "Stored texture identifier is missing");
    require(store.get(7).sample(0, 0) == 0xFF112233U, "Stored texture data changed");

    wolf::loadWallTextures(store);
    wolf::loadSpriteTextures(store);
    require(store.get(wolf::resource::brickWall).valid(), "Brick wall texture is invalid");
    require(store.get(wolf::resource::metalWall).valid(), "Metal wall texture is invalid");
    require(store.get(wolf::resource::stoneWall).valid(), "Stone wall texture is invalid");
    require(store.get(wolf::resource::barrel).sample(0, 0) == 0,
            "Sprite color key should become transparent");
    require(store.get(wolf::resource::lamp).sample(3, 1) != 0,
            "Lamp foreground should remain visible");

    const wolf::Map map = wolf::Map::loadFromFile("assets/wolf/maps/map01.txt");
    requireReadError([] { (void)wolf::Map::loadFromFile("tests/fixtures/oversized_map.txt"); },
                     "Invalid map dimensions:");
    requireReadError([&] { (void)wolf::Scene::loadFromFile("tests/fixtures/truncated_scene.txt", map); },
                     "Malformed sprite data in scene:");
    requireReadError([&] { (void)wolf::Scene::loadFromFile("tests/fixtures/outside_scene.txt", map); },
                     "Invalid sprite in scene:");
    const wolf::Scene scene = wolf::Scene::loadFromFile("assets/wolf/maps/scene01.txt", map);
    require(scene.sprites().size() >= 3, "Demo scene needs several visible objects");
    for (const wolf::Sprite& sprite : scene.sprites())
    {
        require(store.contains(sprite.textureId), "Scene references a missing texture");
    }
}

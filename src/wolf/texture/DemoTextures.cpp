#include "wolf/texture/DemoTextures.hpp"

#include "wolf/texture/ResourceIds.hpp"
#include "wolf/texture/TextureStore.hpp"

namespace wolf
{

void loadWallTextures(TextureStore& store)
{
    store.loadPpm(resource::brickWall, "assets/wolf/textures/wall_brick.ppm");
    store.loadPpm(resource::metalWall, "assets/wolf/textures/wall_metal.ppm");
    store.loadPpm(resource::stoneWall, "assets/wolf/textures/wall_stone.ppm");
}

void loadSpriteTextures(TextureStore& store)
{
    constexpr Texture::Pixel transparent = 0xFFFF00FFU;
    store.loadPpm(resource::barrel, "assets/wolf/sprites/barrel.ppm", transparent);
    store.loadPpm(resource::lamp, "assets/wolf/sprites/lamp.ppm", transparent);
}

} // namespace wolf

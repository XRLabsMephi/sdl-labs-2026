#include "wolf/render/SceneRenderer.hpp"

#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/TexturedWallRenderer.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"

namespace wolf {

void SceneRenderer::drawFrame(FrameBuffer& frame, const Raycaster& raycaster,
                              const Player& player, std::span<const Sprite> sprites,
                              const TextureLookup& textures, const FogSettings& fog) {
    TexturedWallRenderer::drawFrame(frame, raycaster.columns(), textures, fog);
    SpriteRenderer::drawSprites(frame, player, sprites, raycaster.depthBuffer(), textures, fog);
}

} // namespace wolf

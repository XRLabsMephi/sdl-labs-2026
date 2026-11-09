#include "wolf/effects/Fog.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SceneRenderer.hpp"
#include "wolf/render/TexturedWallRenderer.hpp"
#include "wolf/world/Scene.hpp"
#include "wolf/world/SceneResources.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <set>
#include <span>
#include <stdexcept>
#include <vector>

namespace {

    constexpr int frameWidth = 480;
    constexpr int frameHeight = 300;

    void require(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    wolf::Player cameraAt(wolf::Vec2 position, double degrees) {
        const double angle = degrees * std::acos(-1.0) / 180.0;
        wolf::Player player;
        player.position = position;
        player.direction = {std::cos(angle), std::sin(angle)};
        player.cameraPlane = wolf::cameraPlaneFor(player.direction);
        return player;
    }

    bool sameColumn(const wolf::FrameBuffer& a, const wolf::FrameBuffer& b, int x) {
        for (int y = 0; y < a.height(); ++y) {
            if (a.pixelAt(x, y) != b.pixelAt(x, y)) {
                return false;
            }
        }
        return true;
    }

    struct SpriteExtent {
        double depth = 0.0;
        double left = 0.0;
        double right = 0.0;
    };

    SpriteExtent extentOf(
            const wolf::Player& player,
            const wolf::Sprite& sprite,
            const wolf::Texture& texture) {
        const wolf::CameraPoint camera = wolf::toCameraSpace(player, sprite.position);
        const double height = wolf::projectedSize(camera.depth, wolf::focalLength(player, frameWidth)) *
                              sprite.scale;
        const double width = height * texture.width / texture.height;
        const double center = wolf::screenXForCameraX(camera.cameraX, frameWidth);
        return {camera.depth, center - width / 2.0, center + width / 2.0};
    }

    class SceneFixture {
        public:
            SceneFixture()
                : scene_(wolf::loadDefaultScene(wolf::defaultResourceDirectory())),
                  textures_([this](int id) -> const wolf::Texture& { return scene_.textures.get(id); }) {}

            [[nodiscard]] const wolf::Scene& scene() const noexcept {
                return scene_;
            }

            [[nodiscard]] const wolf::TextureLookup& textures() const noexcept {
                return textures_;
            }

        private:
            wolf::Scene scene_;
            wolf::TextureLookup textures_;
    };

    void testTexturedWalls(const SceneFixture& fixture) {
        const wolf::FogSettings noFog{1e9, 2e9, wolf::makeColor(0, 0, 0)};
        std::set<int> wallIds;
        int checkedPixels = 0;
        const wolf::Vec2 positions[] = {{1.5, 1.5}, {10.5, 8.5}, {2.5, 5.5}};
        const double angles[] = {0.0, 200.0, -60.0};
        for (int view = 0; view < 3; ++view) {
            const auto player = cameraAt(positions[view], angles[view]);
            wolf::Raycaster raycaster;
            raycaster.castFrame(fixture.scene().map, player, frameWidth, frameHeight);
            wolf::FrameBuffer frame(frameWidth, frameHeight);
            wolf::SceneRenderer::drawFrame(frame, raycaster, player, {}, fixture.textures(), noFog);
            for (int x = 0; x < frameWidth; ++x) {
                const auto& slice = raycaster.columns()[static_cast<std::size_t>(x)];
                require(slice.hit.hit, "The enclosed default map must have a wall in every column");
                wallIds.insert(slice.hit.wallId);
                if (!wolf::isXSide(slice.hit.side)) {
                    continue;
                }
                const wolf::Texture& texture = fixture.scene().textures.get(slice.hit.wallId);
                const int textureX = wolf::wallTextureX(slice.hit.wallU, texture.width);
                for (int y = slice.drawStart; y < slice.drawEnd; ++y) {
                    const int textureY = wolf::wallTextureY(slice, y, frameHeight, texture.height);
                    require(frame.pixelAt(x, y) == texture.sample(textureX, textureY),
                            "Each wall pixel must be the texel chosen by wallU and textureY");
                    ++checkedPixels;
                }
            }
        }
        require(checkedPixels > 10000, "The textured wall check must cover many pixels");
        require(wallIds.size() == 3, "The demo views must show all three wall textures");
    }

    void testSpritesRespectWallDepth(const SceneFixture& fixture) {
        const auto& scene = fixture.scene();
        const auto player = cameraAt({1.5, 1.5}, 0.0);
        wolf::Raycaster raycaster;
        raycaster.castFrame(scene.map, player, frameWidth, frameHeight);
        wolf::FrameBuffer walls(frameWidth, frameHeight);
        wolf::TexturedWallRenderer::drawFrame(walls, raycaster.columns(), fixture.textures());
        wolf::FrameBuffer full(frameWidth, frameHeight);
        wolf::SceneRenderer::drawFrame(full, raycaster, player, scene.sprites, fixture.textures());

        int changedColumns = 0;
        for (int x = 0; x < frameWidth; ++x) {
            if (sameColumn(walls, full, x)) {
                continue;
            }
            ++changedColumns;
            bool covered = false;
            for (const auto& sprite : scene.sprites) {
                const auto extent = extentOf(player, sprite, scene.textures.get(sprite.textureId));
                covered = covered || (extent.depth > 0.0 &&
                                      extent.depth < raycaster.depthBuffer()[static_cast<std::size_t>(x)] &&
                                      x + 1 >= extent.left && x <= extent.right);
            }
            require(covered, "A sprite may only change columns where it is closer than the wall");
        }
        require(changedColumns > 20, "A sprite in front of a wall must be visible");
    }

    void testPartiallyHiddenItem(const SceneFixture& fixture) {
        const auto& scene = fixture.scene();
        const auto crate = std::find_if(scene.sprites.begin(), scene.sprites.end(), [](const auto& sprite) {
            return sprite.textureId == wolf::sceneTexture::crate;
        });
        require(crate != scene.sprites.end(), "The default scene must contain the crate");
        const auto player = cameraAt({1.5, 1.5}, 0.0);
        wolf::Raycaster raycaster;
        raycaster.castFrame(scene.map, player, frameWidth, frameHeight);
        wolf::FrameBuffer walls(frameWidth, frameHeight);
        wolf::TexturedWallRenderer::drawFrame(walls, raycaster.columns(), fixture.textures());
        wolf::FrameBuffer withCrate(frameWidth, frameHeight);
        const std::span<const wolf::Sprite> onlyCrate(&*crate, 1);
        wolf::SceneRenderer::drawFrame(withCrate, raycaster, player, onlyCrate, fixture.textures());

        const auto extent = extentOf(player, *crate, scene.textures.get(crate->textureId));
        int hidden = 0;
        int visible = 0;
        const int first = std::max(0, static_cast<int>(std::ceil(extent.left)));
        const int last = std::min(frameWidth - 1, static_cast<int>(std::floor(extent.right)) - 1);
        for (int x = first; x <= last; ++x) {
            if (raycaster.depthBuffer()[static_cast<std::size_t>(x)] <= extent.depth) {
                require(sameColumn(walls, withCrate, x), "A wall in front of the crate must hide it");
                ++hidden;
            } else if (!sameColumn(walls, withCrate, x)) {
                ++visible;
            }
        }
        require(hidden > 0 && visible > 0, "The crate must be partially hidden by the metal wall");
    }

    void testFogDependsOnDepth(const SceneFixture& fixture) {
        const auto& scene = fixture.scene();
        const auto player = cameraAt({1.5, 1.5}, 0.0);
        wolf::Raycaster raycaster;
        raycaster.castFrame(scene.map, player, frameWidth, frameHeight);
        wolf::FrameBuffer fogged(frameWidth, frameHeight);
        wolf::TexturedWallRenderer::drawFrame(fogged, raycaster.columns(), fixture.textures());
        wolf::FrameBuffer clear(frameWidth, frameHeight);
        const wolf::FogSettings noFog{1e9, 2e9, wolf::makeColor(0, 0, 0)};
        wolf::TexturedWallRenderer::drawFrame(clear, raycaster.columns(), fixture.textures(), noFog);

        const wolf::FogSettings defaults;
        int nearColumns = 0;
        int farColumns = 0;
        for (int x = 0; x < frameWidth; ++x) {
            const double depth = raycaster.depthBuffer()[static_cast<std::size_t>(x)];
            if (depth <= defaults.start) {
                require(sameColumn(fogged, clear, x), "Walls closer than fog start must keep their color");
                ++nearColumns;
            } else if (depth > defaults.start + 2.0) {
                require(!sameColumn(fogged, clear, x), "Distant walls must fade into the fog");
                ++farColumns;
            }
        }
        require(nearColumns > 0 && farColumns > 0, "The start view must contain near and distant walls");
    }

}

int main() {
    try {
        const SceneFixture fixture;
        testTexturedWalls(fixture);
        testSpritesRespectWallDepth(fixture);
        testPartiallyHiddenItem(fixture);
        testFogDependsOnDepth(fixture);
        std::cout << "Lab 2 scene tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

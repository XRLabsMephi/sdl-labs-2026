#include "wolf/world/Scene.hpp"

#include "wolf/world/SceneResources.hpp"

namespace wolf {

    Scene loadDefaultScene(const std::filesystem::path& resourceDirectory) {
        validateDefaultScene(resourceDirectory);
        Scene scene;
        for (const auto& resource : defaultSceneResources()) {
            scene.textures.load(resource.textureId, resourceDirectory / resource.relativePath);
        }
        placeDefaultItems([&](Vec2 position, int textureId, double scale) {
            (void)scene.textures.get(textureId);
            scene.sprites.push_back({position, textureId, scale});
        });
        return scene;
    }

}

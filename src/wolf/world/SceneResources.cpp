#include "wolf/world/SceneResources.hpp"

#include "wolf/world/Map.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>

namespace wolf {

    namespace {

        constexpr std::array resources{
            SceneResource{sceneTexture::brick, "textures/brick.bmp"},
            SceneResource{sceneTexture::metal, "textures/metal.bmp"},
            SceneResource{sceneTexture::stone, "textures/stone.bmp"},
            SceneResource{sceneTexture::barrel, "sprites/barrel.bmp"},
            SceneResource{sceneTexture::lamp, "sprites/lamp.bmp"},
            SceneResource{sceneTexture::crate, "sprites/crate.bmp"},
        };

        bool hasResource(int id) {
            return std::any_of(resources.begin(),
                               resources.end(),
                               [id](const auto& resource) {
                                    return resource.textureId == id;
                               });
        }

    }

    std::span<const SceneResource> defaultSceneResources() noexcept {
        return resources;
    }

    std::filesystem::path defaultResourceDirectory() {
        const char* basePath = SDL_GetBasePath();
        if (!basePath) {
            throw std::runtime_error(std::string("SDL_GetBasePath: ") + SDL_GetError());
        }
        return std::filesystem::path(basePath) / "assets" / "wolf";
    }

    void placeDefaultItems(const std::function<void(Vec2, int, double)>& placeItem) {
        placeItem({3.5, 1.5}, sceneTexture::barrel, 0.7);
        placeItem({6.0, 1.5}, sceneTexture::lamp, 0.8);
        placeItem({8.5, 2.05}, sceneTexture::crate, 1.2);
        placeItem({5.5, 6.5}, sceneTexture::barrel, 0.9);
    }

    void validateDefaultScene(const std::filesystem::path& resourceDirectory) {
        std::set<int> ids;
        for (const auto& resource : resources) {
            if (resource.textureId <= 0 || !ids.insert(resource.textureId).second) {
                throw std::runtime_error("Invalid or duplicate scene texture ID: " +
                                         std::to_string(resource.textureId));
            }
            const auto path = resourceDirectory / resource.relativePath;
            if (!std::filesystem::is_regular_file(path)) {
                throw std::runtime_error("Scene resource not found: " + path.string());
            }
        }
        const Map map = makeDefaultMap();
        for (int y = 0; y < map.height(); ++y) {
            for (int x = 0; x < map.width(); ++x) {
                const int id = *map.cellAt(x, y);
                if (id > 0 && !hasResource(id)) {
                    throw std::runtime_error("Missing wall texture ID: " + std::to_string(id));
                }
            }
        }
        placeDefaultItems([&](Vec2 position, int textureId, double scale) {
            if (!isFinite(position) ||
                position.x < 0 ||
                position.x >= map.width() ||
                position.y < 0 ||
                position.y >= map.height() ||
                !map.isEmpty(static_cast<int>(std::floor(position.x)),
                             static_cast<int>(std::floor(position.y)))) {
                throw std::runtime_error("Scene item must be in a free map cell");
            }
            if (!hasResource(textureId) ||
                !std::isfinite(scale) ||
                scale <= 0) {
                throw std::runtime_error("Invalid scene item texture ID or scale");
            }
        });
    }

}

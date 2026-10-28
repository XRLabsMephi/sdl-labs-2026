#pragma once

#include "wolf/core/Vec2.hpp"

#include <filesystem>
#include <functional>
#include <span>
#include <string_view>

namespace wolf {

    namespace sceneTexture {

        inline constexpr int brick = 1;
        inline constexpr int metal = 2;
        inline constexpr int stone = 3;
        inline constexpr int barrel = 101;
        inline constexpr int lamp = 102;
        inline constexpr int crate = 103;

    }

    struct SceneResource {
        int textureId;
        std::string_view relativePath;
    };

    [[nodiscard]] std::span<const SceneResource> defaultSceneResources() noexcept;
    [[nodiscard]] std::filesystem::path defaultResourceDirectory();

    void placeDefaultItems(const std::function<void(Vec2, int, double)>& placeItem);
    void validateDefaultScene(const std::filesystem::path& resourceDirectory);

}

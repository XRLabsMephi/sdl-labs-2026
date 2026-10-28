#include "wolf/world/SceneResources.hpp"

#include "wolf/raycast/Raycaster.hpp"
#include "wolf/texture/BmpLoader.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

void testResources() {
    const auto directory = wolf::defaultResourceDirectory();
    require(directory.is_absolute(), "Resources must resolve from the executable, independently of cwd");
    wolf::validateDefaultScene(directory);
    const auto resources = wolf::defaultSceneResources();
    require(resources.size() == 6, "The scene must list three wall textures and three item images");
    std::set<int> ids;
    std::set<std::string> paths;
    for (const auto& resource : resources) {
        require(ids.insert(resource.textureId).second, "Resource IDs must be unique");
        require(paths.insert(std::string(resource.relativePath)).second, "Resource files must be distinct");
        require(!std::filesystem::path(resource.relativePath).is_absolute(), "Manifest paths must be relative");
        int width = 0, height = 0;
        const auto pixels = wolf::loadBmpPixels(directory / resource.relativePath, width, height);
        require(width == 32 && height == 32 && pixels.size() == 1024, "Resource BMP dimensions must match");
        require(std::set<wolf::Color>(pixels.begin(), pixels.end()).size() > 1,
                "Demo textures must have a visible pattern");
        if (resource.textureId <= 3) {
            require(std::all_of(pixels.begin(), pixels.end(), [](auto c) { return (c & 255u) == 255u; }),
                    "Wall resources must be opaque");
        } else {
            require(std::any_of(pixels.begin(), pixels.end(), [](auto c) { return (c & 255u) == 0; }) &&
                        std::any_of(pixels.begin(), pixels.end(), [](auto c) { return (c & 255u) == 255u; }),
                    "Item BMPs must retain transparent background and opaque details");
            if (resource.textureId == wolf::sceneTexture::lamp) {
                require(std::any_of(pixels.begin(), pixels.end(), [](auto c) { return (c & 255u) == 128u; }),
                        "The lamp resource must retain partial alpha");
            }
        }
    }
    const auto map = wolf::makeDefaultMap();
    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            const int id = *map.cellAt(x, y);
            require(id == 0 || ids.contains(id), "Every map wall ID must have a resource");
        }
    }
    const auto absent = directory / "absent-resource-directory";
    bool failed = false;
    try { wolf::validateDefaultScene(absent); }
    catch (const std::runtime_error& error) {
        failed = std::string(error.what()).find(absent.string()) != std::string::npos;
    }
    require(failed, "Missing resource diagnostics must contain the path");
}

void testPlacement() {
    const auto map = wolf::makeDefaultMap();
    const wolf::Player player;
    wolf::Raycaster raycaster;
    constexpr int width = 480;
    raycaster.castFrame(map, player, width, 300);
    int itemCount = 0, visibleItems = 0, partialItems = 0;
    double minDepth = std::numeric_limits<double>::infinity(), maxDepth = 0;
    wolf::placeDefaultItems([&](wolf::Vec2 position, int textureId, double scale) {
        ++itemCount;
        require(isFinite(position) && position.x >= 0 && position.x < map.width() &&
                    position.y >= 0 && position.y < map.height() &&
                    map.isEmpty(static_cast<int>(std::floor(position.x)), static_cast<int>(std::floor(position.y))),
                "Every item must be placed in a free map cell");
        require(textureId >= 101 && textureId <= 103 && std::isfinite(scale) && scale > 0,
                "Item images and scales must be valid");
        // For the initial +X camera, compare rays with the item's world-space width.
        // This checks placement geometry without implementing a SpriteRenderer.
        const double depth = position.x - player.position.x;
        int open = 0, blocked = 0;
        for (int x = 0; x < width; ++x) {
            const auto ray = wolf::rayDirection(player, wolf::cameraXForColumn(x, width));
            const double y = player.position.y + ray.y * depth / ray.x;
            if (std::abs(y - position.y) <= 0.5 * scale) {
                if (depth < raycaster.depthBuffer()[static_cast<std::size_t>(x)]) { ++open; }
                else { ++blocked; }
            }
        }
        if (open > 0) {
            ++visibleItems;
            minDepth = std::min(minDepth, depth);
            maxDepth = std::max(maxDepth, depth);
        }
        if (open > 0 && blocked > 0) { ++partialItems; }
        if (textureId == wolf::sceneTexture::crate) {
            require(open > 0 && blocked > 0, "The crate position must straddle the wall's visible edge");
        }
    });
    require(itemCount == 4 && visibleItems >= 3 && partialItems >= 1 && maxDepth - minDepth > 4,
            "Placement must support items in front of walls, partial occlusion and different depths");
}

} // namespace

int main() {
    try {
        testResources();
        testPlacement();
        std::cout << "All scene resource tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

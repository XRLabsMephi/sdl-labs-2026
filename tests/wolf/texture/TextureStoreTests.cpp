#include "wolf/texture/TextureStore.hpp"
#include "BmpFixture.hpp"

#include <iostream>
#include <type_traits>
#include <utility>

namespace {

using namespace bmp_test;

static_assert(std::is_same_v<decltype(std::declval<const wolf::TextureStore&>().get(1)),
                             const wolf::Texture&>);

template <typename Exception, typename Function>
void requireFailure(Function&& action, const std::string& fragment) {
    try { action(); }
    catch (const Exception& error) {
        require(std::string(error.what()).find(fragment) != std::string::npos,
                "TextureStore error must identify the ID or file");
        return;
    }
    throw std::runtime_error("Expected TextureStore failure: " + fragment);
}

void testStorage(const std::filesystem::path& directory) {
    const std::array<wolf::Color, 6> colors{
        0x11223344u, 0xaabbcc00u, 0xff0000ffu, 0x00ff0080u, 0x0000ffffu, 0xfedcba01u};
    const auto path = directory / "known-pixels.bmp";
    writeBmp(path, true, colors);
    wolf::TextureStore store;
    require(store.size() == 0 && !store.contains(7), "A new store must be empty");
    store.load(7, path);
    const wolf::Texture& texture = store.get(7);
    require(texture.width == 3 && texture.height == 2 && texture.pixels.size() == colors.size(),
            "TextureStore must retain BMP dimensions and pixels");
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 3; ++x) {
            require(texture.sample(x, y) == colors[static_cast<std::size_t>(y * 3 + x)],
                    "Stored textures must preserve all color and alpha channels");
        }
    }
    store.load(12, path);
    std::filesystem::remove(path);
    require(&store.get(7) == &texture && store.get(12).sample(0, 0) == colors[0],
            "Textures must own their pixels and be returned without copying");
    require(store.size() == 2 && store.contains(7) && store.contains(12) && !store.contains(99),
            "Texture IDs must identify exactly the loaded resources");
    requireFailure<std::out_of_range>([&] { (void)store.get(99); }, "99");
    requireFailure<std::invalid_argument>([&] { store.load(7, path); }, "Duplicate texture ID: 7");
    require(store.size() == 2 && store.get(7).sample(0, 0) == colors[0],
            "Duplicate IDs must not overwrite stored data");
}

void testLoadErrors(const std::filesystem::path& directory) {
    wolf::TextureStore store;
    const auto missing = directory / "missing.bmp";
    requireFailure<std::runtime_error>([&] { store.load(3, missing); }, missing.string());
    const auto corrupt = directory / "corrupt.bmp";
    { std::ofstream file(corrupt); file << "Not a BMP"; }
    requireFailure<std::runtime_error>([&] { store.load(4, corrupt); }, corrupt.string());
    requireFailure<std::invalid_argument>([&] { store.load(0, missing); }, "positive: 0");
    requireFailure<std::invalid_argument>([&] { store.load(-2, missing); }, "positive: -2");
    require(store.size() == 0 && !store.contains(3) && !store.contains(4),
            "Failed loads must not publish a partial texture");
    writeBmp(missing, false, {});
    store.load(3, missing);
    require(store.size() == 1 && store.get(3).sample(0, 0) == 0xffu,
            "A failed ID must remain available for a successful load");
}

} // namespace

int main() {
    try {
        const TemporaryDirectory directory;
        testStorage(directory.path);
        testLoadErrors(directory.path);
        std::cout << "All TextureStore tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

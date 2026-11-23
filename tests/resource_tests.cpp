#include "wolf/texture/Texture.hpp"
#include "wolf/texture/TextureStore.hpp"

#include <stdexcept>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
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
}

#pragma once

#include "wolf/texture/Texture.hpp"

#include <functional>

namespace wolf {

// One dependency shared by both CPU renderers: an ID-to-texture lookup.
// The application can adapt its TextureStore without coupling renderers to SDL.
using TextureLookup = std::function<const Texture&(int)>;

} // namespace wolf

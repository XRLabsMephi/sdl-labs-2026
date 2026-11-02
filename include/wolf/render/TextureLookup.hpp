#pragma once

#include "wolf/texture/Texture.hpp"

#include <functional>

namespace wolf {

    using TextureLookup = std::function<const Texture&(int)>;

}

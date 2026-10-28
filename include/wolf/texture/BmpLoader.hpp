#pragma once

#include "wolf/core/Color.hpp"

#include <filesystem>
#include <vector>

namespace wolf {

// Returns owned 0xRRGGBBAA pixels, top row first. Dimensions change only on success.
[[nodiscard]] std::vector<Color> loadBmpPixels(
    const std::filesystem::path& path, int& width, int& height);

} // namespace wolf

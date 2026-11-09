#pragma once

#include "wolf/core/Color.hpp"

#include <filesystem>
#include <vector>

namespace wolf {

    [[nodiscard]] std::vector<Color> loadBmpPixels(const std::filesystem::path& path, int& width, int& height);

}

#include "wolf/render/SoftwareRenderer.hpp"

#include "wolf/core/Color.hpp"
#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace wolf {
namespace {

constexpr Color ceilingColor = makeColor(58, 65, 82);
constexpr Color floorColor = makeColor(63, 56, 52);

[[nodiscard]] constexpr Color wallColor(int wallId) noexcept {
    switch (wallId) {
        case 1: return makeColor(174, 75, 63);
        case 2: return makeColor(83, 134, 170);
        case 3: return makeColor(175, 156, 105);
        default: return makeColor(146, 120, 163);
    }
}

[[nodiscard]] constexpr Color darkenWall(Color color) noexcept {
    // Darken horizontal faces, preserving the original alpha channel.
    return makeColor(
        static_cast<std::uint8_t>(((color >> 24) & 0xffu) * 3u / 4u),
        static_cast<std::uint8_t>(((color >> 16) & 0xffu) * 3u / 4u),
        static_cast<std::uint8_t>(((color >> 8) & 0xffu) * 3u / 4u),
        static_cast<std::uint8_t>(color & 0xffu));
}

} // namespace

void SoftwareRenderer::drawFrame(FrameBuffer& frame,
                                 const std::vector<WallSlice>& columns) noexcept {
    const int horizon = frame.height() / 2;
    frame.clear(floorColor);
    frame.fillRows(0, horizon, ceilingColor);

    const auto count = std::min(columns.size(), static_cast<std::size_t>(frame.width()));
    for (std::size_t x = 0; x < count; ++x) {
        const WallSlice& slice = columns[x];
        if (!slice.hit.hit || slice.hit.wallId <= 0) { continue; }
        Color color = wallColor(slice.hit.wallId);
        if (!isXSide(slice.hit.side)) { color = darkenWall(color); }
        frame.drawVerticalSpan(static_cast<int>(x), slice.drawStart, slice.drawEnd, color);
    }
}

} // namespace wolf

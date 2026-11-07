#pragma once

#include <algorithm>
#include <cstdint>
#include "framebuffer.h"
#include "settings.h"

namespace fog {
	constexpr double fogStart = 4.0 * settings::map_scale;
	constexpr double fogEnd = 18.0 * settings::map_scale;
	constexpr Color fogColor{20, 25, 35, 255};

	inline Color apply(Color color, double depth) {
		double t = std::clamp(
			(depth - fogStart) / (fogEnd - fogStart),
			0.0,
			1.0
		);

		auto mix = [t](std::uint8_t a, std::uint8_t b) {
			return static_cast<std::uint8_t>(
				a * (1.0 - t) + b * t
			);
		};

		return Color{
			mix(color.r, fogColor.r),
			mix(color.g, fogColor.g),
			mix(color.b, fogColor.b),
			color.a
		};
	}
}
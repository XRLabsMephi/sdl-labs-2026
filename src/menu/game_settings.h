#pragma once

#include <array>

struct GameSettings {
	int sensitivityLevel = 1;
	int fovLevel = 1;

	double mouseSensitivity() const {
		return std::array{0.001, 0.002, 0.004}[sensitivityLevel];
	}

	double fovDegrees() const {
		return std::array{70.0, 90.0, 110.0}[fovLevel];
	}
};
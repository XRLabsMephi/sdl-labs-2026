#pragma once
#include "core/Vector2.hpp"

struct Player {
    Vector2 position = {3, 3};
    Vector2 direction = {1, 0};  // всегда единичный
    Vector2 camera_plane = {0, 0.66};  // всегда перпендикулярен direction
};
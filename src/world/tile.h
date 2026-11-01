#pragma once

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/color.h"

inline constexpr bool operator==(const Color& lhs, const Color& rhs) {
  return lhs.GetValue() == rhs.GetValue();
}

inline constexpr bool operator==(const Color& lhs, uint32_t hex_rgb) {
  const uint32_t rgb = (lhs.GetValue() >> 8) & 0xFFFFFF;
  return rgb == hex_rgb;
}

inline constexpr bool operator==(uint32_t hex_rgb, const Color& rhs) {
  return rhs == hex_rgb;
}

inline std::ostream& operator<<(std::ostream& os, const Color& c) {
  return os << "Color(0x" << std::hex << c.GetValue() << std::dec << ")";
}

using TileId = int;

struct TileType {
  TileId id = 0;
  Color color;
  bool is_solid = false;
};

class TileRegistry {
 public:
  void Register(const TileType& tile_type) {
    if (tile_type.id >= static_cast<int>(types_.size())) {
      types_.resize(tile_type.id + 1);
    }
    types_[tile_type.id] = tile_type;
  }

  const TileType& Get(TileId id) const {
    if (id < 0 || id >= static_cast<int>(types_.size())) {
      throw std::out_of_range("TileType not found: " + std::to_string(id));
    }
    return types_[id];
  }

 private:
  std::vector<TileType> types_;
};

struct Tile {
  TileId type_id = 0;

  const TileType& GetType(const TileRegistry& registry) const {
    return registry.Get(type_id);
  }
};

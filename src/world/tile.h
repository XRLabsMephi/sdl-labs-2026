#pragma once

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "core/color.h"
#include "render/texture.h"

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
  Color color = Color::kBlack;
  bool is_solid = false;
  std::string texture_path = "";
  Texture texture = {};
  bool has_texture = false;

  [[nodiscard]] bool HasTexture() const noexcept {
    return has_texture && texture.IsValid();
  }

  [[nodiscard]] const Texture& GetTexture() const noexcept {
    return texture;
  }
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

  bool HasTexture(const TileRegistry& registry) const {
    return GetType(registry).HasTexture();
  }

  const Texture& GetTexture(const TileRegistry& registry) const {
    return GetType(registry).GetTexture();
  }

  const std::string& GetTexturePath(const TileRegistry& registry) const {
    return GetType(registry).texture_path;
  }
};

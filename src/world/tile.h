#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using Color = uint32_t;
using TileId = int;

struct TileType {
  TileId id = 0;
  Color color = 0;
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

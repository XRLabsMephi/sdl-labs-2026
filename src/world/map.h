#pragma once

#include "world/tile.h"

#include <cassert>
#include <string>
#include <vector>

class Map {
public:
  static Map FromFile(const std::string &filename);

  int GetWidth() const noexcept { return width_; }
  int GetHeight() const noexcept { return height_; }

  bool InBounds(int x, int y) const noexcept {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
  }

  Tile &GetTile(int x, int y) {
    assert(InBounds(x, y));
    return tiles_[y * width_ + x];
  }

  const Tile &GetTile(int x, int y) const {
    assert(InBounds(x, y));
    return tiles_[y * width_ + x];
  }

  const TileRegistry &GetRegistry() const noexcept { return tile_registry_; }

  const TileType &GetTileType(int x, int y) const {
    return GetTile(x, y).GetType(tile_registry_);
  }

  bool IsWall(int x, int y) const {
    if (!InBounds(x, y)) return true;
    return GetTileType(x, y).is_solid;
  }

private:
  TileRegistry tile_registry_;

  int width_;
  int height_;

  std::vector<Tile> tiles_;

  Map() : width_(0), height_(0) {}
};

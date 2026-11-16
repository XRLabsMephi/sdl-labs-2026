#pragma once

#include "world/sprite.h"
#include "world/tile.h"

#include <cassert>
#include <cstddef>
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

  bool HasTexture(int x, int y) const {
    if (!InBounds(x, y)) return false;
    return GetTileType(x, y).HasTexture();
  }

  const Texture &GetTexture(int x, int y) const {
    return GetTileType(x, y).GetTexture();
  }

  const std::string &GetTexturePath(int x, int y) const {
    return GetTileType(x, y).texture_path;
  }

  // Управление предметами сцены (Sprite)
  void AddSprite(const Sprite &sprite) {
    sprites_.push_back(sprite);
  }

  void ClearSprites() noexcept {
    sprites_.clear();
  }

  const std::vector<Sprite> &GetSprites() const noexcept {
    return sprites_;
  }

  std::vector<Sprite> &GetSprites() noexcept {
    return sprites_;
  }

  size_t GetSpriteCount() const noexcept {
    return sprites_.size();
  }

private:
  TileRegistry tile_registry_;

  int width_;
  int height_;

  std::vector<Tile> tiles_;
  std::vector<Sprite> sprites_;

  Map() : width_(0), height_(0) {}
};

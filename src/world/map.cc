#include "map.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "render/texture_store.h"

namespace {

bool TryParseHexColor(const std::string& token, uint32_t& out_color) {
  if (token.find('.') != std::string::npos ||
      token.find('/') != std::string::npos ||
      token.find('\\') != std::string::npos) {
    return false;
  }

  std::string hex_str = token;
  if (hex_str.rfind("0x", 0) == 0 || hex_str.rfind("0X", 0) == 0) {
    hex_str = hex_str.substr(2);
  }

  if (hex_str.length() != 6 && hex_str.length() != 8) {
    return false;
  }

  for (char c : hex_str) {
    if (!std::isxdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }

  try {
    size_t idx = 0;
    const unsigned long val = std::stoul(hex_str, &idx, 16);
    if (idx == hex_str.length()) {
      out_color = static_cast<uint32_t>(val);
      return true;
    }
  } catch (...) {
    return false;
  }

  return false;
}

}  // namespace

Map Map::FromFile(const std::string& filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open map file: " + filename);
  }

  Map map;

  map.tile_registry_.Register({0, Color::kBlack, false});

  int tile_type_count = 0;
  if (!(file >> tile_type_count)) {
    throw std::runtime_error("Failed to read tile type count from map file: " +
                             filename);
  }

  for (int i = 0; i < tile_type_count; ++i) {
    TileType tile_type;
    std::string token;
    if (!(file >> tile_type.id >> token)) {
      throw std::runtime_error("Failed to read tile type definition at index " +
                               std::to_string(i));
    }

    uint32_t raw_color = 0;
    if (TryParseHexColor(token, raw_color)) {
      const uint8_t r = static_cast<uint8_t>((raw_color >> 16) & 0xFF);
      const uint8_t g = static_cast<uint8_t>((raw_color >> 8) & 0xFF);
      const uint8_t b = static_cast<uint8_t>(raw_color & 0xFF);
      tile_type.color = Color(r, g, b);
      tile_type.texture = TextureStore::CreateSolid(1, 1, tile_type.color);
      tile_type.has_texture = false;
    } else {
      tile_type.texture_path = token;

      std::string resolved_path = token;
      if (!std::filesystem::exists(resolved_path)) {
        const std::filesystem::path map_dir =
            std::filesystem::path(filename).parent_path();
        if (!map_dir.empty()) {
          if (std::filesystem::exists(map_dir / token)) {
            resolved_path = (map_dir / token).string();
          } else if (map_dir.has_parent_path() &&
                     std::filesystem::exists(map_dir.parent_path() / token)) {
            resolved_path = (map_dir.parent_path() / token).string();
          }
        }
      }

      tile_type.texture = TextureStore::LoadFromFile(resolved_path);
      tile_type.has_texture = true;
      tile_type.color = tile_type.texture.Sample(tile_type.texture.width / 2,
                                                 tile_type.texture.height / 2);
    }

    tile_type.is_solid = (tile_type.id != 0);
    map.tile_registry_.Register(tile_type);
  }

  file >> std::dec;

  if (!(file >> map.width_ >> map.height_) || map.width_ <= 0 ||
      map.height_ <= 0) {
    throw std::runtime_error("Invalid map dimensions in file: " + filename);
  }

  map.tiles_.resize(map.width_ * map.height_);

  std::string line;
  std::getline(file, line);

  for (int y = 0; y < map.height_; ++y) {
    while (std::getline(file, line) && line.empty()) {
    }

    if (file.fail() && line.empty()) {
      throw std::runtime_error("Premature end of file while reading row " +
                               std::to_string(y) + " in " + filename);
    }

    std::vector<int> row_values;
    std::istringstream iss(line);
    int val = 0;
    while (iss >> val) {
      row_values.push_back(val);
    }

    if (static_cast<int>(row_values.size()) == map.width_) {
      for (int x = 0; x < map.width_; ++x) {
        int tile_type_id = row_values[x];
        map.tile_registry_.Get(tile_type_id);
        map.tiles_[y * map.width_ + x].type_id = tile_type_id;
      }
    } else {
      for (int x = 0; x < map.width_; ++x) {
        char ch = (x < static_cast<int>(line.size())) ? line[x] : ' ';
        int tile_type_id = 0;
        if (ch >= '0' && ch <= '9') {
          tile_type_id = ch - '0';
        } else if (ch != ' ') {
          tile_type_id = static_cast<unsigned char>(ch);
        }

        map.tile_registry_.Get(tile_type_id);
        map.tiles_[y * map.width_ + x].type_id = tile_type_id;
      }
    }
  }

  // Опциональная секция спрайтов в конце файла карты
  int sprite_count = 0;
  if (file >> sprite_count && sprite_count > 0) {
    for (int i = 0; i < sprite_count; ++i) {
      std::string sprite_line;
      while (std::getline(file, sprite_line) && sprite_line.empty()) {
      }
      if (!sprite_line.empty()) {
        std::istringstream siss(sprite_line);
        float sx = 0.0f;
        float sy = 0.0f;
        int tex_id = 0;
        double scale = 1.0;
        if (siss >> sx >> sy >> tex_id) {
          siss >> scale;
          map.sprites_.emplace_back(Vec2f{sx, sy}, tex_id, scale);
        }
      }
    }
  }

  return map;
}

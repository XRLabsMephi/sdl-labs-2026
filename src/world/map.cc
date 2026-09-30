#include "map.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

Map Map::FromFile(const std::string& filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open map file: " + filename);
  }

  Map map;

  map.tile_registry_.Register({0, 0x000000, false});

  int tile_type_count = 0;
  if (!(file >> tile_type_count)) {
    throw std::runtime_error("Failed to read tile type count from map file: " +
                             filename);
  }

  for (int i = 0; i < tile_type_count; ++i) {
    TileType tile_type;
    if (!(file >> tile_type.id >> std::hex >> tile_type.color)) {
      throw std::runtime_error("Failed to read tile type definition at index " +
                               std::to_string(i));
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

  return map;
}

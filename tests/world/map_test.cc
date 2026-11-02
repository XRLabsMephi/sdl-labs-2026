#include "world/map.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "render/texture_store.h"

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "tests/test_data"
#endif

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif

class MapTest : public ::testing::Test {
 protected:
  void SetUp() override {
    temp_dir_ = std::filesystem::temp_directory_path() / "map_test_tmp";
    std::filesystem::create_directories(temp_dir_);
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(temp_dir_, ec);
  }

  const std::string test_data_dir = TEST_DATA_DIR;
  const std::string project_root_dir = PROJECT_ROOT_DIR;
  std::filesystem::path temp_dir_;
};

TEST_F(MapTest, ThrowsOnNonExistentFile) {
  EXPECT_THROW(Map::FromFile("non_existent_map_file_12345.map"), std::runtime_error);
}

TEST_F(MapTest, ThrowsOnInvalidDimensions) {
  EXPECT_THROW(Map::FromFile(test_data_dir + "/invalid_dims.map"), std::runtime_error);
}

TEST_F(MapTest, ThrowsOnUnregisteredTile) {
  EXPECT_THROW(Map::FromFile(test_data_dir + "/unregistered_tile.map"), std::out_of_range);
}

TEST_F(MapTest, LoadsTemplateMap) {
  Map map = Map::FromFile(test_data_dir + "/valid_ascii.map");

  // Verify dimensions
  EXPECT_EQ(map.GetWidth(), 6);
  EXPECT_EQ(map.GetHeight(), 6);

  // Row 0: "111111" - North wall (color FFF5F5)
  for (int x = 0; x < 6; ++x) {
    EXPECT_EQ(map.GetTile(x, 0).type_id, 1);
    EXPECT_TRUE(map.IsWall(x, 0));
    EXPECT_EQ(map.GetTileType(x, 0).color, 0xFFF5F5);
    EXPECT_TRUE(map.GetTileType(x, 0).is_solid);
    EXPECT_FALSE(map.HasTexture(x, 0));
  }

  // Row 1: "2    4" - West wall (2: F7D6D0), empty spaces (0), East wall (4: 4A4A4A)
  EXPECT_EQ(map.GetTile(0, 1).type_id, 2);
  EXPECT_TRUE(map.IsWall(0, 1));
  EXPECT_EQ(map.GetTileType(0, 1).color, 0xF7D6D0);

  for (int x = 1; x <= 4; ++x) {
    EXPECT_EQ(map.GetTile(x, 1).type_id, 0);
    EXPECT_FALSE(map.IsWall(x, 1));
    EXPECT_FALSE(map.GetTileType(x, 1).is_solid);
  }

  EXPECT_EQ(map.GetTile(5, 1).type_id, 4);
  EXPECT_TRUE(map.IsWall(5, 1));
  EXPECT_EQ(map.GetTileType(5, 1).color, 0x4A4A4A);

  // Row 2 & 3: "2 1  4" - Contains pillar of type 1 at x = 2
  for (int y = 2; y <= 3; ++y) {
    EXPECT_EQ(map.GetTile(0, y).type_id, 2);
    EXPECT_EQ(map.GetTile(1, y).type_id, 0);
    EXPECT_EQ(map.GetTile(2, y).type_id, 1); // Pillar
    EXPECT_TRUE(map.IsWall(2, y));
    EXPECT_EQ(map.GetTile(3, y).type_id, 0);
    EXPECT_EQ(map.GetTile(4, y).type_id, 0);
    EXPECT_EQ(map.GetTile(5, y).type_id, 4);
  }

  // Row 5: "333333" - South wall (color E2B4BD)
  for (int x = 0; x < 6; ++x) {
    EXPECT_EQ(map.GetTile(x, 5).type_id, 3);
    EXPECT_TRUE(map.IsWall(x, 5));
    EXPECT_EQ(map.GetTileType(x, 5).color, 0xE2B4BD);
  }
}

TEST_F(MapTest, LoadsSpaceSeparatedNumericMap) {
  Map map = Map::FromFile(test_data_dir + "/valid_numeric.map");

  EXPECT_EQ(map.GetWidth(), 4);
  EXPECT_EQ(map.GetHeight(), 3);

  // Row 0: 1 1 1 1
  for (int x = 0; x < 4; ++x) {
    EXPECT_EQ(map.GetTile(x, 0).type_id, 1);
    EXPECT_TRUE(map.IsWall(x, 0));
  }

  // Row 1: 1 0 2 1
  EXPECT_EQ(map.GetTile(0, 1).type_id, 1);
  EXPECT_EQ(map.GetTile(1, 1).type_id, 0);
  EXPECT_FALSE(map.IsWall(1, 1));
  EXPECT_EQ(map.GetTile(2, 1).type_id, 2);
  EXPECT_TRUE(map.IsWall(2, 1));
  EXPECT_EQ(map.GetTileType(2, 1).color, 0x00FF00);
  EXPECT_EQ(map.GetTile(3, 1).type_id, 1);
}

TEST_F(MapTest, BoundaryAndWallHandling) {
  Map map = Map::FromFile(test_data_dir + "/valid_ascii.map");

  // In-bounds checks
  EXPECT_TRUE(map.InBounds(0, 0));
  EXPECT_TRUE(map.InBounds(5, 5));
  EXPECT_FALSE(map.InBounds(-1, 0));
  EXPECT_FALSE(map.InBounds(0, -1));
  EXPECT_FALSE(map.InBounds(6, 0));
  EXPECT_FALSE(map.InBounds(0, 6));

  // Raycasting safety: out of bounds queries must be treated as solid walls
  EXPECT_TRUE(map.IsWall(-1, 2));
  EXPECT_TRUE(map.IsWall(100, 2));
  EXPECT_TRUE(map.IsWall(2, -5));
  EXPECT_TRUE(map.IsWall(2, 100));
}

TEST_F(MapTest, FlyweightPatternIntegrity) {
  Map map = Map::FromFile(test_data_dir + "/valid_ascii.map");

  // All tiles with type_id 1 must share the exact same intrinsic properties
  const TileType& type_at_0_0 = map.GetTileType(0, 0);
  const TileType& type_at_2_2 = map.GetTileType(2, 2);

  EXPECT_EQ(type_at_0_0.id, type_at_2_2.id);
  EXPECT_EQ(type_at_0_0.color, type_at_2_2.color);
  EXPECT_EQ(type_at_0_0.is_solid, type_at_2_2.is_solid);

  // Address in registry should be identical (shared intrinsic state)
  EXPECT_EQ(&type_at_0_0, &type_at_2_2);
}

TEST_F(MapTest, LoadsMapWithTexturePath) {
  const std::filesystem::path bmp_path = temp_dir_ / "brick.bmp";
  Texture brick(4, 4, Color::kRed);
  ASSERT_TRUE(TextureStore::SaveToBmp(brick, bmp_path.string()));

  const std::filesystem::path map_path = temp_dir_ / "textured.map";
  std::ofstream out(map_path);
  out << "1\n";
  out << "1 " << bmp_path.string() << "\n";
  out << "2 2\n";
  out << "1 1\n";
  out << "1 0\n";
  out.close();

  Map map = Map::FromFile(map_path.string());
  EXPECT_EQ(map.GetWidth(), 2);
  EXPECT_EQ(map.GetHeight(), 2);

  EXPECT_TRUE(map.HasTexture(0, 0));
  EXPECT_TRUE(map.GetTileType(0, 0).HasTexture());
  EXPECT_EQ(map.GetTexturePath(0, 0), bmp_path.string());
  EXPECT_EQ(map.GetTexture(0, 0).width, 4);
  EXPECT_EQ(map.GetTexture(0, 0).height, 4);
  EXPECT_EQ(map.GetTexture(0, 0).Sample(0, 0).GetValue(), Color::kRed.GetValue());

  // Empty tile (0) has no texture
  EXPECT_FALSE(map.HasTexture(1, 1));
}

TEST_F(MapTest, ThrowsOnMissingTextureFile) {
  const std::filesystem::path map_path = temp_dir_ / "missing_tex.map";
  std::ofstream out(map_path);
  out << "1\n";
  out << "1 non_existent_texture_file_98765.bmp\n";
  out << "2 2\n";
  out << "1 1\n";
  out << "1 1\n";
  out.close();

  EXPECT_THROW((void)Map::FromFile(map_path.string()), std::runtime_error);
}

TEST_F(MapTest, LoadsMixedColorsAndTextures) {
  const std::filesystem::path bmp_path = temp_dir_ / "stone.bmp";
  Texture stone(2, 2, Color::kBlue);
  ASSERT_TRUE(TextureStore::SaveToBmp(stone, bmp_path.string()));

  const std::filesystem::path map_path = temp_dir_ / "mixed.map";
  std::ofstream out(map_path);
  out << "2\n";
  out << "1 FF0000\n";
  out << "2 " << bmp_path.string() << "\n";
  out << "2 2\n";
  out << "1 2\n";
  out << "0 0\n";
  out.close();

  Map map = Map::FromFile(map_path.string());
  // Tile 1: Color
  EXPECT_FALSE(map.HasTexture(0, 0));
  EXPECT_EQ(map.GetTileType(0, 0).color, 0xFF0000);

  // Tile 2: Texture
  EXPECT_TRUE(map.HasTexture(1, 0));
  EXPECT_EQ(map.GetTexture(1, 0).width, 2);
  EXPECT_EQ(map.GetTexture(1, 0).Sample(0, 0).GetValue(), Color::kBlue.GetValue());
}

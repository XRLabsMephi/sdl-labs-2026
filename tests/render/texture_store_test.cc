#include "render/texture_store.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

class TextureStoreTest : public ::testing::Test {
 protected:
  void SetUp() override {
    temp_dir_ = std::filesystem::temp_directory_path() / "texture_store_tests";
    std::filesystem::create_directories(temp_dir_);
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(temp_dir_, ec);
  }

  std::filesystem::path temp_dir_;
};

TEST_F(TextureStoreTest, TextureDefaultInitialization) {
  Texture tex;
  EXPECT_EQ(tex.width, 0);
  EXPECT_EQ(tex.height, 0);
  EXPECT_TRUE(tex.pixels.empty());
  EXPECT_FALSE(tex.IsValid());
  EXPECT_EQ(tex.sample(0, 0), 0u);
  EXPECT_EQ(tex.Sample(0, 0).GetValue(), Color::kBlack.GetValue());
}

TEST_F(TextureStoreTest, TextureSolidColorConstruction) {
  Texture tex(4, 4, Color::kRed);
  EXPECT_EQ(tex.width, 4);
  EXPECT_EQ(tex.height, 4);
  EXPECT_EQ(tex.pixels.size(), 16u);
  EXPECT_TRUE(tex.IsValid());

  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      EXPECT_EQ(tex.sample(x, y), Color::kRed.GetValue());
      EXPECT_EQ(tex.Sample(x, y).GetValue(), Color::kRed.GetValue());
    }
  }
}

TEST_F(TextureStoreTest, TextureSampleClamping) {
  Texture tex(2, 2, std::vector<uint32_t>{
      Color::kRed.GetValue(), Color::kGreen.GetValue(),
      Color::kBlue.GetValue(), Color::kWhite.GetValue()
  });

  EXPECT_EQ(tex.Sample(0, 0).GetValue(), Color::kRed.GetValue());
  EXPECT_EQ(tex.Sample(1, 0).GetValue(), Color::kGreen.GetValue());
  EXPECT_EQ(tex.Sample(0, 1).GetValue(), Color::kBlue.GetValue());
  EXPECT_EQ(tex.Sample(1, 1).GetValue(), Color::kWhite.GetValue());

  // Clamping negative coordinates to (0, 0) -> Red
  EXPECT_EQ(tex.Sample(-5, -10).GetValue(), Color::kRed.GetValue());
  EXPECT_EQ(tex.Sample(-1, 0).GetValue(), Color::kRed.GetValue());

  // Clamping excessive coordinates to (1, 1) -> White
  EXPECT_EQ(tex.Sample(10, 10).GetValue(), Color::kWhite.GetValue());
  EXPECT_EQ(tex.Sample(1, 10).GetValue(), Color::kWhite.GetValue());

  // Clamping mixed coordinates
  EXPECT_EQ(tex.Sample(-2, 1).GetValue(), Color::kBlue.GetValue());
  EXPECT_EQ(tex.Sample(1, -2).GetValue(), Color::kGreen.GetValue());
}

TEST_F(TextureStoreTest, TextureSampleUV) {
  Texture tex(2, 2, std::vector<uint32_t>{
      Color::kRed.GetValue(), Color::kGreen.GetValue(),
      Color::kBlue.GetValue(), Color::kWhite.GetValue()
  });

  EXPECT_EQ(tex.SampleUV(0.0, 0.0).GetValue(), Color::kRed.GetValue());
  EXPECT_EQ(tex.SampleUV(0.9, 0.0).GetValue(), Color::kGreen.GetValue());
  EXPECT_EQ(tex.SampleUV(0.0, 0.9).GetValue(), Color::kBlue.GetValue());
  EXPECT_EQ(tex.SampleUV(0.9, 0.9).GetValue(), Color::kWhite.GetValue());

  // Out of range UV values clamped
  EXPECT_EQ(tex.SampleUV(-0.5, -0.5).GetValue(), Color::kRed.GetValue());
  EXPECT_EQ(tex.SampleUV(2.0, 2.0).GetValue(), Color::kWhite.GetValue());
}

TEST_F(TextureStoreTest, RegisterAndGetById) {
  TextureStore store;
  EXPECT_TRUE(store.Empty());
  EXPECT_EQ(store.Size(), 0u);

  Texture brick(8, 8, Color::kRed);
  Texture stone(8, 8, Color::kBlue);

  store.Register(1, brick);
  store.Register(2, stone);

  EXPECT_FALSE(store.Empty());
  EXPECT_EQ(store.Size(), 2u);
  EXPECT_TRUE(store.Has(1));
  EXPECT_TRUE(store.Has(2));
  EXPECT_FALSE(store.Has(3));

  EXPECT_EQ(store.Get(1), brick);
  EXPECT_EQ(store.Get(2), stone);

  // Overwriting existing id
  Texture new_brick(8, 8, Color::kGreen);
  store.Register(1, new_brick);
  EXPECT_EQ(store.Size(), 2u);
  EXPECT_EQ(store.Get(1), new_brick);
}

TEST_F(TextureStoreTest, RegisterAutoAssignsId) {
  TextureStore store;
  int id1 = store.Register(Texture(4, 4, Color::kRed));
  int id2 = store.Register(Texture(4, 4, Color::kGreen));

  EXPECT_NE(id1, id2);
  EXPECT_TRUE(store.Has(id1));
  EXPECT_TRUE(store.Has(id2));
  EXPECT_EQ(store.Get(id1).Sample(0, 0).GetValue(), Color::kRed.GetValue());
  EXPECT_EQ(store.Get(id2).Sample(0, 0).GetValue(), Color::kGreen.GetValue());
}

TEST_F(TextureStoreTest, ThrowsOnMissingId) {
  TextureStore store;
  EXPECT_THROW((void)store.Get(42), std::out_of_range);
}

TEST_F(TextureStoreTest, FindReturnsPointerOrNullptr) {
  TextureStore store;
  Texture tex(4, 4, Color::kBlue);
  store.Register(5, tex);

  EXPECT_NE(store.Find(5), nullptr);
  EXPECT_EQ(*store.Find(5), tex);
  EXPECT_EQ(store.Find(99), nullptr);
}

TEST_F(TextureStoreTest, FallbackTextureHandling) {
  TextureStore store;
  EXPECT_FALSE(store.HasFallback());
  EXPECT_THROW((void)store.Get(123), std::out_of_range);

  Texture fallback(4, 4, Color::kWhite);
  store.SetFallback(fallback);
  EXPECT_TRUE(store.HasFallback());

  // Now Get on missing ID should return fallback
  EXPECT_EQ(store.Get(123), fallback);

  // Still returns real texture if present
  Texture real(4, 4, Color::kBlack);
  store.Register(10, real);
  EXPECT_EQ(store.Get(10), real);

  // Clearing fallback
  store.ClearFallback();
  EXPECT_FALSE(store.HasFallback());
  EXPECT_THROW((void)store.Get(123), std::out_of_range);
}

TEST_F(TextureStoreTest, CreateCheckerboard) {
  Texture checker = TextureStore::CreateCheckerboard(4, 4, 2, Color::kBlack, Color::kWhite);
  EXPECT_EQ(checker.width, 4);
  EXPECT_EQ(checker.height, 4);
  EXPECT_TRUE(checker.IsValid());

  // Block (0,0)-(1,1) is Black
  EXPECT_EQ(checker.Sample(0, 0).GetValue(), Color::kBlack.GetValue());
  EXPECT_EQ(checker.Sample(1, 1).GetValue(), Color::kBlack.GetValue());

  // Block (2,0)-(3,1) is White
  EXPECT_EQ(checker.Sample(2, 0).GetValue(), Color::kWhite.GetValue());
  EXPECT_EQ(checker.Sample(3, 1).GetValue(), Color::kWhite.GetValue());

  // Block (0,2)-(1,3) is White
  EXPECT_EQ(checker.Sample(0, 2).GetValue(), Color::kWhite.GetValue());
  EXPECT_EQ(checker.Sample(1, 3).GetValue(), Color::kWhite.GetValue());

  // Block (2,2)-(3,3) is Black
  EXPECT_EQ(checker.Sample(2, 2).GetValue(), Color::kBlack.GetValue());
  EXPECT_EQ(checker.Sample(3, 3).GetValue(), Color::kBlack.GetValue());
}

TEST_F(TextureStoreTest, CreateSolid) {
  Texture solid = TextureStore::CreateSolid(3, 3, Color::kGreen);
  EXPECT_EQ(solid.width, 3);
  EXPECT_EQ(solid.height, 3);
  EXPECT_TRUE(solid.IsValid());
  for (uint32_t p : solid.pixels) {
    EXPECT_EQ(p, Color::kGreen.GetValue());
  }
}

TEST_F(TextureStoreTest, LoadFromFileThrowsOnNonExistentFile) {
  EXPECT_THROW((void)TextureStore::LoadFromFile("non_existent_file_path_xyz.bmp"),
               std::runtime_error);

  TextureStore store;
  EXPECT_THROW(store.Load(1, "non_existent_file_path_xyz.bmp"),
               std::runtime_error);
}

TEST_F(TextureStoreTest, TryLoadReturnsFalseOnBadFile) {
  TextureStore store;
  EXPECT_FALSE(store.TryLoad(1, "non_existent_file_path_xyz.bmp"));
  EXPECT_FALSE(store.Has(1));
}

TEST_F(TextureStoreTest, SaveAndLoadBmpRoundtrip) {
  const std::string file_path = (temp_dir_ / "test_wall.bmp").string();

  // Create a 4x4 texture with distinctive colors
  std::vector<uint32_t> original_pixels = {
      Color::kRed.GetValue(),   Color::kGreen.GetValue(), Color::kBlue.GetValue(),  Color::kWhite.GetValue(),
      Color::kWhite.GetValue(), Color::kBlue.GetValue(),  Color::kGreen.GetValue(), Color::kRed.GetValue(),
      Color::kBlack.GetValue(), Color::kRed.GetValue(),   Color::kWhite.GetValue(), Color::kGreen.GetValue(),
      Color::kBlue.GetValue(),  Color::kWhite.GetValue(), Color::kRed.GetValue(),   Color::kBlack.GetValue()
  };
  Texture original(4, 4, original_pixels);

  EXPECT_TRUE(TextureStore::SaveToBmp(original, file_path));
  EXPECT_TRUE(std::filesystem::exists(file_path));

  // Load back using static method
  Texture loaded = TextureStore::LoadFromFile(file_path);
  EXPECT_EQ(loaded.width, original.width);
  EXPECT_EQ(loaded.height, original.height);
  EXPECT_EQ(loaded.pixels.size(), original.pixels.size());
  EXPECT_EQ(loaded, original);

  // Load into store using TryLoad
  TextureStore store;
  EXPECT_TRUE(store.TryLoad(7, file_path));
  EXPECT_TRUE(store.Has(7));
  EXPECT_EQ(store.Get(7), original);
}

TEST_F(TextureStoreTest, ClearAndSize) {
  TextureStore store;
  store.Register(1, Texture(2, 2, Color::kRed));
  store.Register(2, Texture(2, 2, Color::kBlue));
  store.Register(3, Texture(2, 2, Color::kGreen));

  EXPECT_EQ(store.Size(), 3u);
  EXPECT_FALSE(store.Empty());

  store.Clear();
  EXPECT_EQ(store.Size(), 0u);
  EXPECT_TRUE(store.Empty());
  EXPECT_FALSE(store.Has(1));
}

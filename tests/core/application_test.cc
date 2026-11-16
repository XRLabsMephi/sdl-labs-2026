#include "core/application.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>

class ApplicationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    root_ = std::filesystem::temp_directory_path() /
        ("wolf_app_" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root_ / "maps");
    std::filesystem::create_directories(root_ / "assets/wolf/sprites");
    std::filesystem::copy_file(
        std::filesystem::path(PROJECT_ROOT_DIR) / "assets/wolf/sprites/barrel.bmp",
        root_ / "assets/wolf/sprites/barrel.bmp");
    config_.width = 64;
    config_.height = 48;
    config_.map_file = (root_ / "maps/test.map").string();
  }

  void TearDown() override { std::filesystem::remove_all(root_); }

  void WriteScene(bool sprite = false, bool occluded = false,
                  bool textured = false) {
    std::ofstream out(config_.map_file);
    out << "1\n1 ";
    if (textured) {
      const auto path = root_ / "wall.bmp";
      ASSERT_TRUE(TextureStore::SaveToBmp(Texture::FromColor(2, 2, Color::kRed),
                                         path.string()));
      out << path.string();
    } else {
      out << "FF0000";
    }
    out << "\n14 5\n11111111111111\n";
    out << (occluded ? "1 1          1\n" : "1            1\n");
    out << "1            1\n1            1\n11111111111111\n";
    if (sprite) out << "1\n3.5 1.5 10 1\n";
  }

  Color Center(const Application& app) {
    return Color(app.GetFrameBuffer().GetPixelData()[24 * 64 + 32]);
  }

  Config config_;
  std::filesystem::path root_;
};

TEST_F(ApplicationTest, AppliesFogToColoredWallAndFillsDepthBuffer) {
  WriteScene();
  Application app(config_);
  app.RenderFrame();
  ASSERT_EQ(app.GetDepthBuffer().size(), 64u);
  EXPECT_NEAR(app.GetDepthBuffer()[32], 11.5, 1e-5);
  EXPECT_EQ(Center(app), Fog().Apply(Color::kRed, 11.5));
  EXPECT_TRUE(PlayerController().CanMoveTo(app.GetMap(), app.GetPlayer().position,
                                          0.2f));
}

TEST_F(ApplicationTest, AppliesSameFogToTexturedWall) {
  WriteScene(false, false, true);
  Application app(config_);
  app.RenderFrame();
  EXPECT_EQ(Center(app), Fog().Apply(Color::kRed, 11.5));
}

TEST_F(ApplicationTest, DrawsLoadedSpriteAfterWalls) {
  WriteScene(true);
  Application app(config_);
  app.RenderFrame();
  const auto& texture = app.GetTextureStore().Get(10);
  const Color texel = texture.Sample(texture.GetWidth() / 2,
                                     texture.GetHeight() / 2);
  ASSERT_FALSE(SpriteRenderer::IsTransparent(texel));
  EXPECT_EQ(Center(app), Fog().Apply(texel, 2.0));
}

TEST_F(ApplicationTest, KeepsSpriteBehindWallHidden) {
  WriteScene(true, true);
  Application app(config_);
  app.RenderFrame();
  EXPECT_NEAR(app.GetDepthBuffer()[32], 0.5, 1e-5);
  EXPECT_EQ(Center(app), Color::kRed);
}

TEST_F(ApplicationTest, RejectsMapWithoutSafeSpawn) {
  std::ofstream(config_.map_file) << "1\n1 FF0000\n2 2\n11\n11\n";
  EXPECT_THROW(Application app(config_), std::runtime_error);
}

TEST_F(ApplicationTest, RejectsUnknownSpriteTexture) {
  WriteScene();
  std::ofstream(config_.map_file, std::ios::app) << "1\n3.5 1.5 999 1\n";
  EXPECT_THROW(Application app(config_), std::runtime_error);
}

TEST_F(ApplicationTest, ConsumesQuitEventBeforeAnotherFrame) {
  WriteScene();
  Application app(config_);
  SDL_Event event{};
  event.type = SDL_EVENT_QUIT;
  ASSERT_TRUE(SDL_PushEvent(&event));
  app.Run();
  EXPECT_EQ(app.GetFrameBuffer().GetPixelData()[0], Color::kBlack.GetValue());
}

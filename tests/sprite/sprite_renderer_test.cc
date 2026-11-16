#include "sprite/sprite_renderer.h"

#include <gtest/gtest.h>

#include <limits>
#include <vector>

namespace {

constexpr double kInf = std::numeric_limits<double>::infinity();

Player MakeCamera() {
  Player player;
  player.position = Vec2f{0.0f, 0.0f};
  player.SetDirection(Vec2f{1.0f, 0.0f}, 0.66f);
  return player;
}

Texture MakeOpaque(Color color) { return Texture::FromColor(4, 4, color); }

Texture MakeKeyed() {
  std::vector<uint32_t> pixels(4, Color::kMagentaKey.GetValue());
  pixels[0] = Color::kWhite.GetValue();
  return Texture(2, 2, std::move(pixels));
}

}  // namespace

TEST(SpriteRendererTest, ProjectsSpriteInFrontOfCamera) {
  const SpriteRenderer renderer;
  const Texture texture = MakeOpaque(Color::kWhite);
  const Sprite sprite{Vec2f{2.0f, 0.0f}, 0, 1.0};
  const SpriteProjection projection =
      renderer.Project(sprite, MakeCamera(), texture, 200, 100);

  ASSERT_TRUE(projection.visible);
  EXPECT_NEAR(projection.depth, 2.0, 1e-5);
  EXPECT_NEAR(projection.screen_x, 100.0, 1e-4);
  EXPECT_NEAR(projection.sprite_height, 50.0, 1e-4);
  EXPECT_LT(projection.draw_start_x, 100);
  EXPECT_GT(projection.draw_end_x, 100);
}

TEST(SpriteRendererTest, HidesSpriteBehindCamera) {
  const SpriteRenderer renderer;
  const Texture texture = MakeOpaque(Color::kWhite);
  const Sprite sprite{Vec2f{-2.0f, 0.0f}, 0, 1.0};
  const SpriteProjection projection =
      renderer.Project(sprite, MakeCamera(), texture, 200, 100);
  EXPECT_FALSE(projection.visible);
}

TEST(SpriteRendererTest, SkipsTransparentAndOccludedPixels) {
  FrameBuffer frame_buffer(8, 8);
  frame_buffer.Clear(Color(1, 1, 1), Color(1, 1, 1));
  const Color background = Color(frame_buffer.GetPixelData()[0]);

  const SpriteRenderer renderer;
  const std::vector<Texture> textures = {MakeKeyed()};
  const Sprite sprite{Vec2f{1.5f, 0.0f}, 0, 1.0};
  const Fog fog(100.0, 200.0, Color::kFog);

  std::vector<double> hidden(8, 0.1);
  renderer.Draw({sprite}, MakeCamera(), textures, hidden, frame_buffer, fog);
  EXPECT_EQ(Color(frame_buffer.GetPixelData()[4 * 8 + 4]), background);

  std::vector<double> open(8, kInf);
  renderer.Draw({sprite}, MakeCamera(), textures, open, frame_buffer, fog);

  bool saw_white = false;
  bool saw_magenta = false;
  const uint32_t* pixels = frame_buffer.GetPixelData();
  for (int i = 0; i < 64; ++i) {
    const Color color(pixels[i]);
    saw_white = saw_white || color == Color::kWhite;
    saw_magenta = saw_magenta || color == Color::kMagentaKey;
  }
  EXPECT_TRUE(saw_white);
  EXPECT_FALSE(saw_magenta);
}

TEST(SpriteRendererTest, DrawsNearerSpriteOverFartherOne) {
  FrameBuffer frame_buffer(16, 16);
  frame_buffer.Clear(Color::kBlack, Color::kBlack);

  const SpriteRenderer renderer;
  const std::vector<Texture> textures = {MakeOpaque(Color(255, 0, 0)),
                                         MakeOpaque(Color(0, 255, 0))};
  const std::vector<Sprite> sprites = {
      Sprite{Vec2f{3.0f, 0.0f}, 0, 2.0},
      Sprite{Vec2f{1.5f, 0.0f}, 1, 2.0},
  };
  const std::vector<double> depth_buffer(16, kInf);
  const Fog fog(100.0, 200.0, Color::kFog);
  renderer.Draw(sprites, MakeCamera(), textures, depth_buffer, frame_buffer,
                fog);

  EXPECT_EQ(Color(frame_buffer.GetPixelData()[8 * 16 + 8]), Color(0, 255, 0));
}

TEST(SpriteRendererTest, AppliesFogToSpriteColor) {
  FrameBuffer frame_buffer(16, 16);
  frame_buffer.Clear(Color::kBlack, Color::kBlack);

  const SpriteRenderer renderer;
  const std::vector<Texture> textures = {MakeOpaque(Color(100, 0, 0))};
  const Sprite sprite{Vec2f{2.0f, 0.0f}, 0, 1.0};
  const std::vector<double> depth_buffer(16, kInf);
  const Fog fog(0.0, 4.0, Color(0, 0, 0));
  renderer.Draw({sprite}, MakeCamera(), textures, depth_buffer, frame_buffer,
                fog);

  const Color pixel(frame_buffer.GetPixelData()[8 * 16 + 8]);
  EXPECT_NEAR(pixel.GetR(), 50, 1);
  EXPECT_EQ(pixel.GetG(), 0);
  EXPECT_EQ(pixel.GetB(), 0);
}

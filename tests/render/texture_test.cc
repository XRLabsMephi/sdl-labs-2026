#include "render/texture.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

TEST(TextureTest, RejectsInvalidSize) {
  EXPECT_TRUE(Texture(0, 4, {1, 2, 3, 4}).IsEmpty());
  EXPECT_TRUE(Texture(2, 2, {1, 2, 3}).IsEmpty());
}

TEST(TextureTest, SamplesWrappedCoordinates) {
  const std::vector<uint32_t> pixels = {
      Color(10, 0, 0).GetValue(), Color(20, 0, 0).GetValue(),
      Color(30, 0, 0).GetValue(), Color(40, 0, 0).GetValue()};
  const Texture texture(2, 2, pixels);

  EXPECT_EQ(texture.Sample(0, 0).GetR(), 10);
  EXPECT_EQ(texture.Sample(1, 0).GetR(), 20);
  EXPECT_EQ(texture.Sample(0, 1).GetR(), 30);
  EXPECT_EQ(texture.Sample(1, 1).GetR(), 40);
  EXPECT_EQ(texture.Sample(2, 0).GetR(), 10);
  EXPECT_EQ(texture.Sample(-1, 0).GetR(), 20);
  EXPECT_EQ(texture.Sample(0, -1).GetR(), 30);
}

TEST(TextureTest, FromColorFillsEveryTexel) {
  const Texture texture = Texture::FromColor(3, 2, Color(1, 2, 3));
  ASSERT_EQ(texture.GetWidth(), 3);
  ASSERT_EQ(texture.GetHeight(), 2);
  EXPECT_EQ(texture.Sample(2, 1), Color(1, 2, 3));
}

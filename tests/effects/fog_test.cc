#include "effects/fog.h"

#include <gtest/gtest.h>

#include <limits>

TEST(FogTest, KeepsSceneColorBeforeStart) {
  const Fog fog(2.0, 6.0, Color::kWhite);
  EXPECT_DOUBLE_EQ(fog.Factor(0.0), 0.0);
  EXPECT_DOUBLE_EQ(fog.Factor(2.0), 0.0);
  EXPECT_EQ(fog.Apply(Color(10, 20, 30), 1.0), Color(10, 20, 30));
}

TEST(FogTest, ReachesFogColorAfterEnd) {
  const Fog fog(2.0, 6.0, Color::kWhite);
  EXPECT_DOUBLE_EQ(fog.Factor(6.0), 1.0);
  EXPECT_EQ(fog.Apply(Color(10, 20, 30), 10.0), Color::kWhite);
  EXPECT_DOUBLE_EQ(fog.Factor(std::numeric_limits<double>::infinity()), 1.0);
}

TEST(FogTest, BlendsAtMidpoint) {
  const Fog fog(0.0, 2.0, Color(0, 0, 0));
  EXPECT_NEAR(fog.Factor(1.0), 0.5, 1e-12);
  const Color mixed = fog.Apply(Color(100, 50, 0), 1.0);
  EXPECT_EQ(mixed.GetR(), 50);
  EXPECT_EQ(mixed.GetG(), 25);
  EXPECT_EQ(mixed.GetB(), 0);
  EXPECT_EQ(mixed.GetA(), 255);
}

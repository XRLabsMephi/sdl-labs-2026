#include "core/config.h"
#include "render/frame_buffer.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {
Config Parse(std::vector<std::string> arguments) {
  std::vector<char*> argv;
  for (auto& arg : arguments) argv.push_back(arg.data());
  return Config::Parse(static_cast<int>(argv.size()), argv.data());
}
}

TEST(ConfigTest, ParsesWindowAndMapOptions) {
  const auto config = Parse({"wolf", "--width", "320", "--height", "200",
                              "--map", "maps/default.map", "--fullscreen"});
  EXPECT_EQ(config.width, 320);
  EXPECT_EQ(config.height, 200);
  EXPECT_EQ(config.map_file, "maps/default.map");
  EXPECT_EQ(config.screen_mode, Config::ScreenMode::Fullscreen);
}

TEST(ConfigTest, RejectsInvalidOrMissingArguments) {
  for (const auto& value : {"0", "-1", "12junk", "9999999999999", "8193"}) {
    EXPECT_THROW(Parse({"wolf", "--width", value}), std::exception);
  }
  EXPECT_THROW(Parse({"wolf", "--map"}), std::invalid_argument);
  EXPECT_THROW(Parse({"wolf", "--height"}), std::invalid_argument);
  EXPECT_THROW(Parse({"wolf", "--unknown"}), std::invalid_argument);
  EXPECT_THROW(Parse({"wolf", "--width", "8192", "--height", "8192"}),
               std::invalid_argument);
  EXPECT_THROW(FrameBuffer(-1, 20), std::invalid_argument);
}

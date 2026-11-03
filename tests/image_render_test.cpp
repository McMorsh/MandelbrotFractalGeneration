#include <cstddef>
#include <cstdio>
#include <fstream>
#include <vector>

#include <gtest/gtest.h>
#include <raylib.h>

#include "color_map.hpp"
#include "image_render.hpp"

namespace {

constexpr int kWidth = 16;
constexpr int kHeight = 12;
constexpr int kMaxIterations = 50;
constexpr const char* kPath = "image_render_test.png";

struct FileGuard {
  ~FileGuard() {
    std::remove(kPath);
  }
};

TEST(ImageRenderTest, SavesPngWithSignature) {
  const FileGuard guard;
  const std::vector<double> data(static_cast<std::size_t>(kWidth) * kHeight,
                                 kMaxIterations);
  const ColorMap colormap;

  ASSERT_TRUE(SaveIterationImage(kPath, data, kWidth, kHeight, kMaxIterations,
                                 colormap));

  std::ifstream file(kPath, std::ios::binary);
  ASSERT_TRUE(file);
  unsigned char signature[8]{};
  file.read(reinterpret_cast<char*>(signature), sizeof(signature));
  ASSERT_EQ(file.gcount(), static_cast<std::streamsize>(sizeof(signature)));
  EXPECT_EQ(signature[0], 0x89);
  EXPECT_EQ(signature[1], 'P');
  EXPECT_EQ(signature[2], 'N');
  EXPECT_EQ(signature[3], 'G');
}

TEST(ImageRenderTest, RejectsSizeMismatch) {
  const std::vector<double> data(10, kMaxIterations);
  const ColorMap colormap;

  EXPECT_FALSE(SaveIterationImage(kPath, data, kWidth, kHeight,
                                  kMaxIterationCount + 1, colormap));
  EXPECT_FALSE(SaveIterationImage(kPath, data, kWidth, kHeight + 1,
                                  kMaxIterations, colormap));
}

TEST(ImageRenderTest, RejectsTooLarge) {
  const std::vector<double> data(kMaxImageDimension + 1, kMaxIterations);
  const ColorMap colormap;

  EXPECT_FALSE(SaveIterationImage(kPath, data, kMaxImageDimension + 1, 1,
                                  kMaxIterations, colormap));
}

TEST(ImageRenderTest, RejectsInvalidArguments) {
  const std::vector<double> data(static_cast<std::size_t>(kWidth) * kHeight,
                                 kMaxIterations);
  const ColorMap colormap;

  EXPECT_FALSE(
      SaveIterationImage("", data, kWidth, kHeight, kMaxIterations, colormap));
  EXPECT_FALSE(
      SaveIterationImage(kPath, data, 0, kHeight, kMaxIterations, colormap));
  EXPECT_FALSE(SaveIterationImage(kPath, data, kWidth, kHeight, 0, colormap));
  EXPECT_FALSE(
      SaveIterationImage(kPath, data, kWidth, kHeight, 100001, colormap));
}

}  // namespace

#include <gtest/gtest.h>

#include "export_presets.hpp"

namespace {

TEST(ExportPresetsTest, PresetsUseStandardRatio) {
  for (int i = 0; i < kExportPresetCount; ++i) {
    EXPECT_EQ(kExportPresets[i].width * 9, kExportPresets[i].height * 16);
  }
}

TEST(ExportPresetsTest, SizesGrowWithIndex) {
  for (int i = 1; i < kExportPresetCount; ++i) {
    EXPECT_LT(kExportPresets[i - 1].width, kExportPresets[i].width);
    EXPECT_LT(kExportPresets[i - 1].height, kExportPresets[i].height);
  }
}

TEST(ExportPresetsTest, IndexWrapsBothWays) {
  EXPECT_EQ(AdjustExportPresetIndex(kExportPresetCount - 1, 1), 0);
  EXPECT_EQ(AdjustExportPresetIndex(0, -1), kExportPresetCount - 1);
  EXPECT_EQ(AdjustExportPresetIndex(1, 1), 2);
}

TEST(ExportPresetsTest, FindsIndexByLabel) {
  EXPECT_EQ(GetExportPresetIndex("FHD"), 0);
  EXPECT_EQ(GetExportPresetIndex("8K"), kExportPresetCount - 1);
  EXPECT_EQ(GetExportPresetIndex("unknown"), 0);
}

TEST(ExportPresetsTest, SpanKeepsVerticalExtent) {
  constexpr double kViewSpan = 4.0;
  constexpr int kViewWidth = 1200;
  constexpr int kViewHeight = 800;
  const double visible_height =
      kViewSpan * static_cast<double>(kViewHeight) / kViewWidth;

  for (int i = 0; i < kExportPresetCount; ++i) {
    const ExportPreset& preset = kExportPresets[i];
    const double span =
        ComputeExportSpan(kViewSpan, kViewWidth, kViewHeight, preset);
    const double exported_height =
        span * static_cast<double>(preset.height) / preset.width;

    EXPECT_NEAR(exported_height, visible_height, 1e-12);
  }
}

TEST(ExportPresetsTest, SixteenByNineViewKeepsSpan) {
  EXPECT_NEAR(ComputeExportSpan(2.0, 1920, 1080, kExportPresets[0]), 2.0,
              1e-12);
}

}  // namespace

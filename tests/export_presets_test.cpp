#include "ExportPresets.h"

#include <gtest/gtest.h>

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
		EXPECT_EQ(adjustExportPresetIndex(kExportPresetCount - 1, 1), 0);
		EXPECT_EQ(adjustExportPresetIndex(0, -1), kExportPresetCount - 1);
		EXPECT_EQ(adjustExportPresetIndex(1, 1), 2);
	}

	TEST(ExportPresetsTest, FindsIndexByLabel) {
		EXPECT_EQ(exportPresetIndex("FHD"), 0);
		EXPECT_EQ(exportPresetIndex("8K"), kExportPresetCount - 1);
		EXPECT_EQ(exportPresetIndex("unknown"), 0);
	}

	TEST(ExportPresetsTest, SpanKeepsVerticalExtent) {
		constexpr double kViewSpan = 4.0;
		constexpr int kViewWidth = 1200;
		constexpr int kViewHeight = 800;
		const double visibleHeight = kViewSpan * static_cast<double>(kViewHeight) / kViewWidth;

		for (int i = 0; i < kExportPresetCount; ++i) {
			const ExportPreset& preset = kExportPresets[i];
			const double span = exportSpanFor(kViewSpan, kViewWidth, kViewHeight, preset);
			const double exportedHeight = span * static_cast<double>(preset.height) / preset.width;

			EXPECT_NEAR(exportedHeight, visibleHeight, 1e-12);
		}
	}

	TEST(ExportPresetsTest, SixteenByNineViewKeepsSpan) {
		EXPECT_NEAR(exportSpanFor(2.0, 1920, 1080, kExportPresets[0]), 2.0, 1e-12);
	}

} // namespace

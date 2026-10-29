#include "ColorMap.h"

#include <gtest/gtest.h>

namespace {

	void expectBlack(const Color& color) {
		EXPECT_EQ(color.r, 0);
		EXPECT_EQ(color.g, 0);
		EXPECT_EQ(color.b, 0);
		EXPECT_EQ(color.a, 255);
	}

	TEST(ColorMapTest, InsideSetReturnsBlack) {
		const ColorMap map;

		expectBlack(map.getColor(150.0, 150));
	}

	TEST(ColorMapTest, OvershootReturnsBlack) {
		const ColorMap map;

		expectBlack(map.getColor(151.0, 150));
	}

	TEST(ColorMapTest, ZeroIterationLimitReturnsBlack) {
		const ColorMap map;

		expectBlack(map.getColor(10.0, 0));
	}

	TEST(ColorMapTest, OutsidePointIsOpaque) {
		const ColorMap map;
		const Color color = map.getColor(10.0, 150);

		EXPECT_EQ(color.a, 255);
		EXPECT_TRUE(color.r != 0 || color.g != 0 || color.b != 0);
	}

	TEST(ColorMapTest, ZeroIterationReturnsPaletteStart) {
		const ColorMap map;
		const Color color = map.getColor(0.0, 150);

		EXPECT_EQ(color.a, 255);
		EXPECT_TRUE(color.r != 0 || color.g != 0 || color.b != 0);
	}

	TEST(ColorMapTest, ExtremeValuesAreSafe) {
		const ColorMap map;
		const Color low = map.getColor(-5.0, 150);
		const Color high = map.getColor(149.999, 150);

		EXPECT_EQ(low.a, 255);
		EXPECT_EQ(high.a, 255);
	}

	TEST(ColorMapTest, SetPaletteIsCaseInsensitive) {
		ColorMap map;

		EXPECT_TRUE(map.setPalette("ocean"));
		EXPECT_STREQ(map.paletteName(), "Ocean");
		EXPECT_TRUE(map.setPalette("OCEAN"));
		EXPECT_STREQ(map.paletteName(), "Ocean");
	}

	TEST(ColorMapTest, UnknownPaletteKeepsCurrent) {
		ColorMap map;
		const Color before = map.getColor(10.0, 150);

		EXPECT_FALSE(map.setPalette("Nope"));
		const Color after = map.getColor(10.0, 150);

		EXPECT_EQ(before.r, after.r);
		EXPECT_EQ(before.g, after.g);
		EXPECT_EQ(before.b, after.b);
		EXPECT_STREQ(map.paletteName(), "Ember");
	}

	TEST(ColorMapTest, NextPaletteWraps) {
		ColorMap map;

		EXPECT_STREQ(map.paletteName(), "Ember");
		map.nextPalette();
		EXPECT_STREQ(map.paletteName(), "Ocean");
		map.nextPalette();
		EXPECT_STREQ(map.paletteName(), "Mono");
		map.nextPalette();
		EXPECT_STREQ(map.paletteName(), "Ember");
	}

	TEST(ColorMapTest, PaletteChangeChangesColor) {
		ColorMap map;
		const Color ember = map.getColor(10.0, 150);

		ASSERT_TRUE(map.setPalette("Ocean"));
		const Color ocean = map.getColor(10.0, 150);

		EXPECT_TRUE(ember.r != ocean.r || ember.g != ocean.g || ember.b != ocean.b);
	}

} // namespace

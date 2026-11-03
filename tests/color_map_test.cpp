#include <gtest/gtest.h>
#include <raylib.h>

#include "color_map.hpp"

namespace {

void ExpectBlack(const Color& color) {
  EXPECT_EQ(color.r, 0);
  EXPECT_EQ(color.g, 0);
  EXPECT_EQ(color.b, 0);
  EXPECT_EQ(color.a, 255);
}

TEST(ColorMapTest, InsideSetReturnsBlack) {
  const ColorMap map;

  ExpectBlack(map.GetColor(150.0, 150));
}

TEST(ColorMapTest, OvershootReturnsBlack) {
  const ColorMap map;

  ExpectBlack(map.GetColor(151.0, 150));
}

TEST(ColorMapTest, ZeroIterationLimitReturnsBlack) {
  const ColorMap map;

  ExpectBlack(map.GetColor(10.0, 0));
}

TEST(ColorMapTest, OutsidePointIsOpaque) {
  const ColorMap map;
  const Color color = map.GetColor(10.0, 150);

  EXPECT_EQ(color.a, 255);
  EXPECT_TRUE(color.r != 0 || color.g != 0 || color.b != 0);
}

TEST(ColorMapTest, ZeroIterationReturnsPaletteStart) {
  const ColorMap map;
  const Color color = map.GetColor(0.0, 150);

  EXPECT_EQ(color.a, 255);
  EXPECT_TRUE(color.r != 0 || color.g != 0 || color.b != 0);
}

TEST(ColorMapTest, ExtremeValuesAreSafe) {
  const ColorMap map;
  const Color low = map.GetColor(-5.0, 150);
  const Color high = map.GetColor(149.999, 150);

  EXPECT_EQ(low.a, 255);
  EXPECT_EQ(high.a, 255);
}

TEST(ColorMapTest, SetPaletteIsCaseInsensitive) {
  ColorMap map;

  EXPECT_TRUE(map.SetPalette("ocean"));
  EXPECT_STREQ(map.GetPaletteName(), "Ocean");
  EXPECT_TRUE(map.SetPalette("OCEAN"));
  EXPECT_STREQ(map.GetPaletteName(), "Ocean");
}

TEST(ColorMapTest, UnknownPaletteKeepsCurrent) {
  ColorMap map;
  const Color before = map.GetColor(10.0, 150);

  EXPECT_FALSE(map.SetPalette("Nope"));
  const Color after = map.GetColor(10.0, 150);

  EXPECT_EQ(before.r, after.r);
  EXPECT_EQ(before.g, after.g);
  EXPECT_EQ(before.b, after.b);
  EXPECT_STREQ(map.GetPaletteName(), "Ember");
}

TEST(ColorMapTest, NextPaletteWraps) {
  ColorMap map;

  EXPECT_STREQ(map.GetPaletteName(), "Ember");
  map.SelectNextPalette();
  EXPECT_STREQ(map.GetPaletteName(), "Ocean");
  map.SelectNextPalette();
  EXPECT_STREQ(map.GetPaletteName(), "Mono");
  map.SelectNextPalette();
  EXPECT_STREQ(map.GetPaletteName(), "Ember");
}

TEST(ColorMapTest, PaletteChangeChangesColor) {
  ColorMap map;
  const Color ember = map.GetColor(10.0, 150);

  ASSERT_TRUE(map.SetPalette("Ocean"));
  const Color ocean = map.GetColor(10.0, 150);

  EXPECT_TRUE(ember.r != ocean.r || ember.g != ocean.g || ember.b != ocean.b);
}

}  // namespace

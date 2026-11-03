#include <bit>
#include <cmath>
#include <cstdint>
#include <string>

#include <gtest/gtest.h>

#include "view.hpp"
#include "view_snapshot.hpp"

namespace {

TEST(ViewSnapshotTest, RoundTripPreservesValues) {
  const View view(-0.743643887037151, 0.131825904205330, 1e-6, 1200, 800);
  const std::string text = FormatViewSnapshot(view, 900, "Ocean");

  ViewSnapshot snapshot;
  ASSERT_TRUE(ParseViewSnapshot(text, snapshot));
  EXPECT_DOUBLE_EQ(snapshot.center_x, view.GetCenterX());
  EXPECT_DOUBLE_EQ(snapshot.center_y, view.GetCenterY());
  EXPECT_DOUBLE_EQ(snapshot.span, view.GetSpan());
  EXPECT_EQ(snapshot.iterations, 900);
  EXPECT_EQ(snapshot.palette, "Ocean");
}

TEST(ViewSnapshotTest, RoundTripKeepsExactBits) {
  const double center = 0.1 + 0.2;
  const View view(center, -center, std::nextafter(1.0, 2.0), 1200, 800);
  const std::string text = FormatViewSnapshot(view, 321, "Mono");

  ViewSnapshot snapshot;
  ASSERT_TRUE(ParseViewSnapshot(text, snapshot));
  EXPECT_EQ(std::bit_cast<std::uint64_t>(snapshot.center_x),
            std::bit_cast<std::uint64_t>(center));
  EXPECT_EQ(std::bit_cast<std::uint64_t>(snapshot.center_y),
            std::bit_cast<std::uint64_t>(-center));
  EXPECT_EQ(std::bit_cast<std::uint64_t>(snapshot.span),
            std::bit_cast<std::uint64_t>(view.GetSpan()));
}

TEST(ViewSnapshotTest, ParsesCrlfAndUnknownLines) {
  const std::string text =
      "note\r\n"
      "center (1.5, -2.25i)\r\n"
      "span 0.5\r\n"
      "zoom 7x\r\n"
      "iterations 400\r\n"
      "palette Ocean\r\n"
      "extra";

  ViewSnapshot snapshot;
  ASSERT_TRUE(ParseViewSnapshot(text, snapshot));
  EXPECT_DOUBLE_EQ(snapshot.center_x, 1.5);
  EXPECT_DOUBLE_EQ(snapshot.center_y, -2.25);
  EXPECT_DOUBLE_EQ(snapshot.span, 0.5);
  EXPECT_EQ(snapshot.iterations, 400);
  EXPECT_EQ(snapshot.palette, "Ocean");
}

TEST(ViewSnapshotTest, IgnoresLookalikeLines) {
  const std::string text =
      "centerfold (1, 2i)\n"
      "spanning tree\n"
      "iterations: 100\n"
      "paletteOcean\n"
      "center (1.5, -2.25i)\n"
      "span 0.5\n"
      "iterations 400\n"
      "palette Ocean";

  ViewSnapshot snapshot;
  ASSERT_TRUE(ParseViewSnapshot(text, snapshot));
  EXPECT_DOUBLE_EQ(snapshot.center_x, 1.5);
  EXPECT_DOUBLE_EQ(snapshot.center_y, -2.25);
  EXPECT_DOUBLE_EQ(snapshot.span, 0.5);
  EXPECT_EQ(snapshot.iterations, 400);
  EXPECT_EQ(snapshot.palette, "Ocean");
}

TEST(ViewSnapshotTest, RejectsDuplicateFields) {
  const std::string text =
      "center (0, +0i)\n"
      "center (1, +1i)\n"
      "span 3.5\n"
      "iterations 100\n"
      "palette Ember";

  ViewSnapshot snapshot;
  EXPECT_FALSE(ParseViewSnapshot(text, snapshot));
}

TEST(ViewSnapshotTest, AcceptsSpanBounds) {
  for (const double span : {kMinSpan, kMaxSpan}) {
    const View view(0.0, 0.0, span, 1200, 800);
    const std::string text = FormatViewSnapshot(view, 100, "Ember");

    ViewSnapshot snapshot;
    ASSERT_TRUE(ParseViewSnapshot(text, snapshot)) << "span: " << span;
    EXPECT_EQ(std::bit_cast<std::uint64_t>(snapshot.span),
              std::bit_cast<std::uint64_t>(span));
  }
}

TEST(ViewSnapshotTest, UsesValidFieldAfterMalformed) {
  const std::string text =
      "span abc\n"
      "center (0, +0i)\n"
      "span 3.5\n"
      "iterations 100\n"
      "palette Ember";

  ViewSnapshot snapshot;
  ASSERT_TRUE(ParseViewSnapshot(text, snapshot));
  EXPECT_DOUBLE_EQ(snapshot.span, 3.5);
}

TEST(ViewSnapshotTest, RejectsEmptyText) {
  ViewSnapshot snapshot;

  EXPECT_FALSE(ParseViewSnapshot("", snapshot));
}

TEST(ViewSnapshotTest, RejectsMissingFields) {
  ViewSnapshot snapshot;

  EXPECT_FALSE(
      ParseViewSnapshot("center (0, +0i)\nspan 3.5\niterations 100", snapshot));
  EXPECT_FALSE(
      ParseViewSnapshot("center (0, +0i)\nspan 3.5\npalette Ember", snapshot));
  EXPECT_FALSE(
      ParseViewSnapshot("span 3.5\niterations 100\npalette Ember", snapshot));
}

TEST(ViewSnapshotTest, RejectsInvalidSpan) {
  for (const char* span : {"0", "-1", "nan", "inf", "abc", "1e-12", "100"}) {
    const std::string text = std::string("center (0, +0i)\nspan ") + span +
                             "\niterations 100\npalette Ember";
    ViewSnapshot snapshot;

    EXPECT_FALSE(ParseViewSnapshot(text, snapshot)) << "span: " << span;
  }
}

TEST(ViewSnapshotTest, RejectsInvalidIterations) {
  for (const char* iterations : {"0", "-10", "abc", "1.5"}) {
    const std::string text =
        std::string("center (0, +0i)\nspan 3.5\niterations ") + iterations +
        "\npalette Ember";
    ViewSnapshot snapshot;

    EXPECT_FALSE(ParseViewSnapshot(text, snapshot))
        << "iterations: " << iterations;
  }
}

TEST(ViewSnapshotTest, RejectsInvalidCenter) {
  for (const char* center :
       {"center (1, 2)", "center (nan, 0i)", "center 1 2", "center (, +0i)"}) {
    const std::string text =
        std::string(center) + "\nspan 3.5\niterations 100\npalette Ember";
    ViewSnapshot snapshot;

    EXPECT_FALSE(ParseViewSnapshot(text, snapshot)) << "center: " << center;
  }
}

TEST(ViewSnapshotTest, RejectsEmptyPalette) {
  ViewSnapshot snapshot;

  EXPECT_FALSE(ParseViewSnapshot(
      "center (0, +0i)\nspan 3.5\niterations 100\npalette   ", snapshot));
}

}  // namespace

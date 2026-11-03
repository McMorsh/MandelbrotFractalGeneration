#include <limits>

#include <gtest/gtest.h>

#include "view.hpp"

namespace {

constexpr int kWidth = 1200;
constexpr int kHeight = 800;

TEST(ViewTest, DefaultViewMatchesInitial) {
  const View view(kWidth, kHeight);

  EXPECT_DOUBLE_EQ(view.GetCenterX(), kInitialCenterX);
  EXPECT_DOUBLE_EQ(view.GetCenterY(), kInitialCenterY);
  EXPECT_DOUBLE_EQ(view.GetSpan(), kInitialSpan);
  EXPECT_EQ(view.GetWidth(), kWidth);
  EXPECT_EQ(view.GetHeight(), kHeight);
}

TEST(ViewTest, PointAtCenterReturnsCenter) {
  const View view(kWidth, kHeight);
  const ComplexPoint point = view.GetPointAt(kWidth * 0.5, kHeight * 0.5);

  EXPECT_NEAR(point.real, kInitialCenterX, 1e-12);
  EXPECT_NEAR(point.imag, kInitialCenterY, 1e-12);
}

TEST(ViewTest, PointAtLeftEdgeShiftsByHalfSpan) {
  const View view(kWidth, kHeight);
  const ComplexPoint point = view.GetPointAt(0.0, kHeight * 0.5);

  EXPECT_NEAR(point.real, kInitialCenterX - kInitialSpan * 0.5, 1e-12);
  EXPECT_NEAR(point.imag, kInitialCenterY, 1e-12);
}

TEST(ViewTest, PointAtTopEdgeShiftsByHalfVisibleHeight) {
  const View view(kWidth, kHeight);
  const ComplexPoint point = view.GetPointAt(kWidth * 0.5, 0.0);

  EXPECT_NEAR(point.real, kInitialCenterX, 1e-12);
  EXPECT_NEAR(point.imag,
              kInitialCenterY + kInitialSpan * (kHeight * 0.5) / kWidth, 1e-12);
}

TEST(ViewTest, PointAtBottomEdgeShiftsDown) {
  const View view(kWidth, kHeight);
  const ComplexPoint point = view.GetPointAt(kWidth * 0.5, kHeight);

  EXPECT_NEAR(point.real, kInitialCenterX, 1e-12);
  EXPECT_NEAR(point.imag,
              kInitialCenterY - kInitialSpan * (kHeight * 0.5) / kWidth, 1e-12);
}

TEST(ViewTest, ZoomKeepsPointUnderCursor) {
  View view(kWidth, kHeight);
  const double cursor_x = 900.0;
  const double cursor_y = 300.0;
  const ComplexPoint before = view.GetPointAt(cursor_x, cursor_y);

  ASSERT_TRUE(view.ZoomAt(0.5, cursor_x, cursor_y));

  const ComplexPoint after = view.GetPointAt(cursor_x, cursor_y);
  EXPECT_NEAR(after.real, before.real, 1e-12);
  EXPECT_NEAR(after.imag, before.imag, 1e-12);
  EXPECT_NEAR(view.GetSpan(), kInitialSpan * 0.5, 1e-12);
}

TEST(ViewTest, ZoomKeepsPointUnderCorner) {
  View view(kWidth, kHeight);
  const ComplexPoint before = view.GetPointAt(0.0, 0.0);

  ASSERT_TRUE(view.ZoomAt(2.0, 0.0, 0.0));

  const ComplexPoint after = view.GetPointAt(0.0, 0.0);
  EXPECT_NEAR(after.real, before.real, 1e-12);
  EXPECT_NEAR(after.imag, before.imag, 1e-12);
}

TEST(ViewTest, ZoomStopsAtMinSpan) {
  View view(kInitialCenterX, kInitialCenterY, kMinSpan, kWidth, kHeight);

  EXPECT_FALSE(view.ZoomAt(0.5, 600.0, 400.0));
  EXPECT_DOUBLE_EQ(view.GetSpan(), kMinSpan);
}

TEST(ViewTest, ZoomStopsAtMaxSpan) {
  View view(kInitialCenterX, kInitialCenterY, kMaxSpan, kWidth, kHeight);

  EXPECT_FALSE(view.ZoomAt(2.0, 600.0, 400.0));
  EXPECT_DOUBLE_EQ(view.GetSpan(), kMaxSpan);
}

TEST(ViewTest, ZoomRejectsNonPositiveFactor) {
  View view(kWidth, kHeight);

  EXPECT_FALSE(view.ZoomAt(0.0, 600.0, 400.0));
  EXPECT_FALSE(view.ZoomAt(-1.0, 600.0, 400.0));
  EXPECT_DOUBLE_EQ(view.GetSpan(), kInitialSpan);
}

TEST(ViewTest, ZoomRejectsNaNFactor) {
  View view(kWidth, kHeight);

  EXPECT_FALSE(
      view.ZoomAt(std::numeric_limits<double>::quiet_NaN(), 600.0, 400.0));
  EXPECT_DOUBLE_EQ(view.GetSpan(), kInitialSpan);
}

TEST(ViewTest, ZoomRejectsNaNCoordinates) {
  View view(kWidth, kHeight);

  EXPECT_FALSE(
      view.ZoomAt(0.5, std::numeric_limits<double>::quiet_NaN(), 400.0));
  EXPECT_DOUBLE_EQ(view.GetCenterX(), kInitialCenterX);
  EXPECT_DOUBLE_EQ(view.GetSpan(), kInitialSpan);
}

TEST(ViewTest, ZoomAfterResizeKeepsPointUnderCursor) {
  View view(kWidth, kHeight);
  view.Resize(640, 480);
  const ComplexPoint before = view.GetPointAt(100.0, 80.0);

  ASSERT_TRUE(view.ZoomAt(0.5, 100.0, 80.0));

  const ComplexPoint after = view.GetPointAt(100.0, 80.0);
  EXPECT_NEAR(after.real, before.real, 1e-12);
  EXPECT_NEAR(after.imag, before.imag, 1e-12);
}

TEST(ViewTest, DeepZoomKeepsPointUnderCursor) {
  View view(kInitialCenterX, kInitialCenterY, 1e-9, kWidth, kHeight);
  const double cursor_x = 813.0;
  const double cursor_y = 221.0;
  const ComplexPoint before = view.GetPointAt(cursor_x, cursor_y);

  ASSERT_TRUE(view.ZoomAt(0.25, cursor_x, cursor_y));

  const ComplexPoint after = view.GetPointAt(cursor_x, cursor_y);
  EXPECT_NEAR(after.real, before.real, 1e-14);
  EXPECT_NEAR(after.imag, before.imag, 1e-14);
}

TEST(ViewTest, PanByHalfWidthMovesCenterByHalfSpan) {
  View view(kWidth, kHeight);

  view.PanBy(kWidth * 0.5, 0.0);

  EXPECT_NEAR(view.GetCenterX(), kInitialCenterX - kInitialSpan * 0.5, 1e-12);
  EXPECT_NEAR(view.GetCenterY(), kInitialCenterY, 1e-12);
}

TEST(ViewTest, PanDownMovesCenterByVisibleHeight) {
  View view(kWidth, kHeight);

  view.PanBy(0.0, 100.0);

  EXPECT_NEAR(view.GetCenterX(), kInitialCenterX, 1e-12);
  EXPECT_NEAR(view.GetCenterY(),
              kInitialCenterY + 100.0 * kInitialSpan / kWidth, 1e-12);
}

TEST(ViewTest, PanIgnoresNonFiniteDelta) {
  View view(kWidth, kHeight);

  view.PanBy(std::numeric_limits<double>::infinity(), 0.0);

  EXPECT_DOUBLE_EQ(view.GetCenterX(), kInitialCenterX);
  EXPECT_DOUBLE_EQ(view.GetCenterY(), kInitialCenterY);
}

TEST(ViewTest, ResizeKeepsCenterAndSpan) {
  View view(kWidth, kHeight);

  view.Resize(640, 480);

  EXPECT_EQ(view.GetWidth(), 640);
  EXPECT_EQ(view.GetHeight(), 480);
  EXPECT_DOUBLE_EQ(view.GetCenterX(), kInitialCenterX);
  EXPECT_DOUBLE_EQ(view.GetCenterY(), kInitialCenterY);
  EXPECT_DOUBLE_EQ(view.GetSpan(), kInitialSpan);
}

TEST(ViewTest, ResizeUpdatesVisibleArea) {
  View view(kWidth, kHeight);

  view.Resize(600, 400);

  const ComplexPoint left = view.GetPointAt(0.0, 400.0 * 0.5);
  EXPECT_NEAR(left.real, kInitialCenterX - kInitialSpan * 0.5, 1e-12);
}

TEST(ViewTest, ResizeIgnoresNonPositiveSize) {
  View view(kWidth, kHeight);

  view.Resize(0, 480);
  view.Resize(640, -1);

  EXPECT_EQ(view.GetWidth(), kWidth);
  EXPECT_EQ(view.GetHeight(), kHeight);
}

TEST(ViewTest, SameViewIgnoresScreenSize) {
  const View lhs(kWidth, kHeight);
  const View rhs(640, 480);

  EXPECT_TRUE(IsSameView(lhs, rhs));
}

TEST(ViewTest, SameViewDetectsZoom) {
  const View lhs(kWidth, kHeight);
  View rhs(kWidth, kHeight);
  ASSERT_TRUE(rhs.ZoomAt(0.5, 600.0, 400.0));

  EXPECT_FALSE(IsSameView(lhs, rhs));
}

TEST(ViewTest, SameViewDetectsPan) {
  const View lhs(kWidth, kHeight);
  View rhs(kWidth, kHeight);
  rhs.PanBy(10.0, 0.0);

  EXPECT_FALSE(IsSameView(lhs, rhs));
}

TEST(ViewTest, ConstructorReplacesInvalidSpan) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const View zero(kInitialCenterX, kInitialCenterY, 0.0, kWidth, kHeight);
  const View negative(kInitialCenterX, kInitialCenterY, -1.0, kWidth, kHeight);
  const View not_finite(kInitialCenterX, kInitialCenterY, nan, kWidth, kHeight);

  EXPECT_DOUBLE_EQ(zero.GetSpan(), kInitialSpan);
  EXPECT_DOUBLE_EQ(negative.GetSpan(), kInitialSpan);
  EXPECT_DOUBLE_EQ(not_finite.GetSpan(), kInitialSpan);
}

TEST(ViewTest, ConstructorClampsSpan) {
  const View huge(kInitialCenterX, kInitialCenterY, 100.0, kWidth, kHeight);
  const View tiny(kInitialCenterX, kInitialCenterY, 1e-30, kWidth, kHeight);

  EXPECT_DOUBLE_EQ(huge.GetSpan(), kMaxSpan);
  EXPECT_DOUBLE_EQ(tiny.GetSpan(), kMinSpan);
}

TEST(ViewTest, ConstructorReplacesInvalidCenter) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();
  const View view(nan, inf, kInitialSpan, kWidth, kHeight);

  EXPECT_DOUBLE_EQ(view.GetCenterX(), kInitialCenterX);
  EXPECT_DOUBLE_EQ(view.GetCenterY(), kInitialCenterY);
}

}  // namespace

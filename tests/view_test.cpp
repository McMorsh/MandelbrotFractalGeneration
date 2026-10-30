#include "View.h"

#include <gtest/gtest.h>

#include <limits>

namespace {

	constexpr int kWidth = 1200;
	constexpr int kHeight = 800;

	TEST(ViewTest, DefaultViewIsHome) {
		const View view(kWidth, kHeight);

		EXPECT_DOUBLE_EQ(view.getCenterX(), kInitialCenterX);
		EXPECT_DOUBLE_EQ(view.getCenterY(), kInitialCenterY);
		EXPECT_DOUBLE_EQ(view.getSpan(), kInitialSpan);
		EXPECT_EQ(view.getWidth(), kWidth);
		EXPECT_EQ(view.getHeight(), kHeight);
	}

	TEST(ViewTest, PointAtCenterReturnsCenter) {
		const View view(kWidth, kHeight);
		const ComplexPoint point = view.pointAt(kWidth * 0.5, kHeight * 0.5);

		EXPECT_NEAR(point.real, kInitialCenterX, 1e-12);
		EXPECT_NEAR(point.imag, kInitialCenterY, 1e-12);
	}

	TEST(ViewTest, PointAtLeftEdgeShiftsByHalfSpan) {
		const View view(kWidth, kHeight);
		const ComplexPoint point = view.pointAt(0.0, kHeight * 0.5);

		EXPECT_NEAR(point.real, kInitialCenterX - kInitialSpan * 0.5, 1e-12);
		EXPECT_NEAR(point.imag, kInitialCenterY, 1e-12);
	}

	TEST(ViewTest, PointAtTopEdgeShiftsByHalfVisibleHeight) {
		const View view(kWidth, kHeight);
		const ComplexPoint point = view.pointAt(kWidth * 0.5, 0.0);

		EXPECT_NEAR(point.real, kInitialCenterX, 1e-12);
		EXPECT_NEAR(point.imag, kInitialCenterY + kInitialSpan * (kHeight * 0.5) / kWidth, 1e-12);
	}

	TEST(ViewTest, PointAtBottomEdgeShiftsDown) {
		const View view(kWidth, kHeight);
		const ComplexPoint point = view.pointAt(kWidth * 0.5, kHeight);

		EXPECT_NEAR(point.real, kInitialCenterX, 1e-12);
		EXPECT_NEAR(point.imag, kInitialCenterY - kInitialSpan * (kHeight * 0.5) / kWidth, 1e-12);
	}

	TEST(ViewTest, ZoomKeepsPointUnderCursor) {
		View view(kWidth, kHeight);
		const double cursorX = 900.0;
		const double cursorY = 300.0;
		const ComplexPoint before = view.pointAt(cursorX, cursorY);

		ASSERT_TRUE(view.zoomAt(0.5, cursorX, cursorY));

		const ComplexPoint after = view.pointAt(cursorX, cursorY);
		EXPECT_NEAR(after.real, before.real, 1e-12);
		EXPECT_NEAR(after.imag, before.imag, 1e-12);
		EXPECT_NEAR(view.getSpan(), kInitialSpan * 0.5, 1e-12);
	}

	TEST(ViewTest, ZoomKeepsPointUnderCorner) {
		View view(kWidth, kHeight);
		const ComplexPoint before = view.pointAt(0.0, 0.0);

		ASSERT_TRUE(view.zoomAt(2.0, 0.0, 0.0));

		const ComplexPoint after = view.pointAt(0.0, 0.0);
		EXPECT_NEAR(after.real, before.real, 1e-12);
		EXPECT_NEAR(after.imag, before.imag, 1e-12);
	}

	TEST(ViewTest, ZoomStopsAtMinSpan) {
		View view(kInitialCenterX, kInitialCenterY, kMinSpan, kWidth, kHeight);

		EXPECT_FALSE(view.zoomAt(0.5, 600.0, 400.0));
		EXPECT_DOUBLE_EQ(view.getSpan(), kMinSpan);
	}

	TEST(ViewTest, ZoomStopsAtMaxSpan) {
		View view(kInitialCenterX, kInitialCenterY, kMaxSpan, kWidth, kHeight);

		EXPECT_FALSE(view.zoomAt(2.0, 600.0, 400.0));
		EXPECT_DOUBLE_EQ(view.getSpan(), kMaxSpan);
	}

	TEST(ViewTest, ZoomRejectsNonPositiveFactor) {
		View view(kWidth, kHeight);

		EXPECT_FALSE(view.zoomAt(0.0, 600.0, 400.0));
		EXPECT_FALSE(view.zoomAt(-1.0, 600.0, 400.0));
		EXPECT_DOUBLE_EQ(view.getSpan(), kInitialSpan);
	}

	TEST(ViewTest, ZoomRejectsNaNFactor) {
		View view(kWidth, kHeight);

		EXPECT_FALSE(view.zoomAt(std::numeric_limits<double>::quiet_NaN(), 600.0, 400.0));
		EXPECT_DOUBLE_EQ(view.getSpan(), kInitialSpan);
	}

	TEST(ViewTest, ZoomRejectsNaNCoordinates) {
		View view(kWidth, kHeight);

		EXPECT_FALSE(view.zoomAt(0.5, std::numeric_limits<double>::quiet_NaN(), 400.0));
		EXPECT_DOUBLE_EQ(view.getCenterX(), kInitialCenterX);
		EXPECT_DOUBLE_EQ(view.getSpan(), kInitialSpan);
	}

	TEST(ViewTest, ZoomAfterResizeKeepsPointUnderCursor) {
		View view(kWidth, kHeight);
		view.resize(640, 480);
		const ComplexPoint before = view.pointAt(100.0, 80.0);

		ASSERT_TRUE(view.zoomAt(0.5, 100.0, 80.0));

		const ComplexPoint after = view.pointAt(100.0, 80.0);
		EXPECT_NEAR(after.real, before.real, 1e-12);
		EXPECT_NEAR(after.imag, before.imag, 1e-12);
	}

	TEST(ViewTest, DeepZoomKeepsPointUnderCursor) {
		View view(kInitialCenterX, kInitialCenterY, 1e-9, kWidth, kHeight);
		const double cursorX = 813.0;
		const double cursorY = 221.0;
		const ComplexPoint before = view.pointAt(cursorX, cursorY);

		ASSERT_TRUE(view.zoomAt(0.25, cursorX, cursorY));

		const ComplexPoint after = view.pointAt(cursorX, cursorY);
		EXPECT_NEAR(after.real, before.real, 1e-14);
		EXPECT_NEAR(after.imag, before.imag, 1e-14);
	}

	TEST(ViewTest, PanByHalfWidthMovesCenterByHalfSpan) {
		View view(kWidth, kHeight);

		view.panBy(kWidth * 0.5, 0.0);

		EXPECT_NEAR(view.getCenterX(), kInitialCenterX - kInitialSpan * 0.5, 1e-12);
		EXPECT_NEAR(view.getCenterY(), kInitialCenterY, 1e-12);
	}

	TEST(ViewTest, PanDownMovesCenterByVisibleHeight) {
		View view(kWidth, kHeight);

		view.panBy(0.0, 100.0);

		EXPECT_NEAR(view.getCenterX(), kInitialCenterX, 1e-12);
		EXPECT_NEAR(view.getCenterY(), kInitialCenterY + 100.0 * kInitialSpan / kWidth, 1e-12);
	}

	TEST(ViewTest, PanIgnoresNonFiniteDelta) {
		View view(kWidth, kHeight);

		view.panBy(std::numeric_limits<double>::infinity(), 0.0);

		EXPECT_DOUBLE_EQ(view.getCenterX(), kInitialCenterX);
		EXPECT_DOUBLE_EQ(view.getCenterY(), kInitialCenterY);
	}

	TEST(ViewTest, ResizeKeepsCenterAndSpan) {
		View view(kWidth, kHeight);

		view.resize(640, 480);

		EXPECT_EQ(view.getWidth(), 640);
		EXPECT_EQ(view.getHeight(), 480);
		EXPECT_DOUBLE_EQ(view.getCenterX(), kInitialCenterX);
		EXPECT_DOUBLE_EQ(view.getCenterY(), kInitialCenterY);
		EXPECT_DOUBLE_EQ(view.getSpan(), kInitialSpan);
	}

	TEST(ViewTest, ResizeUpdatesVisibleArea) {
		View view(kWidth, kHeight);

		view.resize(600, 400);

		const ComplexPoint left = view.pointAt(0.0, 400.0 * 0.5);
		EXPECT_NEAR(left.real, kInitialCenterX - kInitialSpan * 0.5, 1e-12);
	}

	TEST(ViewTest, ResizeIgnoresNonPositiveSize) {
		View view(kWidth, kHeight);

		view.resize(0, 480);
		view.resize(640, -1);

		EXPECT_EQ(view.getWidth(), kWidth);
		EXPECT_EQ(view.getHeight(), kHeight);
	}

	TEST(ViewTest, SameViewIgnoresScreenSize) {
		const View lhs(kWidth, kHeight);
		const View rhs(640, 480);

		EXPECT_TRUE(sameView(lhs, rhs));
	}

	TEST(ViewTest, SameViewDetectsZoom) {
		const View lhs(kWidth, kHeight);
		View rhs(kWidth, kHeight);
		ASSERT_TRUE(rhs.zoomAt(0.5, 600.0, 400.0));

		EXPECT_FALSE(sameView(lhs, rhs));
	}

	TEST(ViewTest, SameViewDetectsPan) {
		const View lhs(kWidth, kHeight);
		View rhs(kWidth, kHeight);
		rhs.panBy(10.0, 0.0);

		EXPECT_FALSE(sameView(lhs, rhs));
	}

	TEST(ViewTest, ConstructorReplacesInvalidSpan) {
		const double nan = std::numeric_limits<double>::quiet_NaN();
		const View zero(kInitialCenterX, kInitialCenterY, 0.0, kWidth, kHeight);
		const View negative(kInitialCenterX, kInitialCenterY, -1.0, kWidth, kHeight);
		const View notFinite(kInitialCenterX, kInitialCenterY, nan, kWidth, kHeight);

		EXPECT_DOUBLE_EQ(zero.getSpan(), kInitialSpan);
		EXPECT_DOUBLE_EQ(negative.getSpan(), kInitialSpan);
		EXPECT_DOUBLE_EQ(notFinite.getSpan(), kInitialSpan);
	}

	TEST(ViewTest, ConstructorClampsSpan) {
		const View huge(kInitialCenterX, kInitialCenterY, 100.0, kWidth, kHeight);
		const View tiny(kInitialCenterX, kInitialCenterY, 1e-30, kWidth, kHeight);

		EXPECT_DOUBLE_EQ(huge.getSpan(), kMaxSpan);
		EXPECT_DOUBLE_EQ(tiny.getSpan(), kMinSpan);
	}

	TEST(ViewTest, ConstructorReplacesInvalidCenter) {
		const double nan = std::numeric_limits<double>::quiet_NaN();
		const double inf = std::numeric_limits<double>::infinity();
		const View view(nan, inf, kInitialSpan, kWidth, kHeight);

		EXPECT_DOUBLE_EQ(view.getCenterX(), kInitialCenterX);
		EXPECT_DOUBLE_EQ(view.getCenterY(), kInitialCenterY);
	}

} // namespace

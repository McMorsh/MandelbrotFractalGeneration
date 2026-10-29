#include "Fractal.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <stop_token>
#include <vector>

namespace {

TEST(FractalTest, PointInsideSetReturnsIterationLimit) {
    const Fractal fractal(200);

    EXPECT_DOUBLE_EQ(fractal.compute(0.0, 0.0), 200.0);
}

TEST(FractalTest, LeftBulbInside) {
    const Fractal fractal(200);

    EXPECT_DOUBLE_EQ(fractal.compute(-1.0, 0.0), 200.0);
}

TEST(FractalTest, BoundaryPointStaysInside) {
    const Fractal fractal(200);

    EXPECT_DOUBLE_EQ(fractal.compute(-2.0, 0.0), 200.0);
}

TEST(FractalTest, PointOutsideEscapesQuickly) {
    const Fractal fractal(200);

    EXPECT_LT(fractal.compute(3.0, 0.0), 5.0);
}

TEST(FractalTest, SmoothValueIsNotInteger) {
    const Fractal fractal(200);
    const double value = fractal.compute(3.0, 0.0);

    EXPECT_GT(value, 0.0);
    EXPECT_LT(value, 200.0);
    EXPECT_NE(value, std::floor(value));
}

TEST(FractalTest, GenerateSizeMatchesPixels) {
    const Fractal fractal(100);

    EXPECT_EQ(fractal.generate(64, 48, -0.6, 0.0, 3.5).size(),
              static_cast<std::size_t>(64 * 48));
}

TEST(FractalTest, GenerateCenterIsInside) {
    const Fractal fractal(100);
    constexpr int width = 64;
    constexpr int height = 64;
    const std::vector<double> data = fractal.generate(width, height, -0.6, 0.0, 3.5);

    ASSERT_EQ(data.size(), static_cast<std::size_t>(width * height));
    const std::size_t center = static_cast<std::size_t>(height / 2) * width + width / 2;
    EXPECT_DOUBLE_EQ(data[center], 100.0);
}

TEST(FractalTest, GenerateCornerEscapes) {
    const Fractal fractal(100);
    const std::vector<double> data = fractal.generate(64, 64, -0.6, 0.0, 3.5);

    ASSERT_FALSE(data.empty());
    EXPECT_LT(data.front(), 100.0);
}

TEST(FractalTest, GenerateIsDeterministic) {
    const Fractal fractal(100);
    const std::vector<double> first = fractal.generate(32, 24, -0.6, 0.0, 3.5);
    const std::vector<double> second = fractal.generate(32, 24, -0.6, 0.0, 3.5);

    EXPECT_EQ(first, second);
}

TEST(FractalTest, GenerateRejectsInvalidSize) {
    const Fractal fractal(100);

    EXPECT_TRUE(fractal.generate(0, 10, -0.6, 0.0, 3.5).empty());
    EXPECT_TRUE(fractal.generate(10, -1, -0.6, 0.0, 3.5).empty());
}

TEST(FractalTest, GenerateRejectsInvalidSpan) {
    const Fractal fractal(100);

    EXPECT_TRUE(fractal.generate(10, 10, -0.6, 0.0, 0.0).empty());
    EXPECT_TRUE(fractal.generate(10, 10, -0.6, 0.0, -1.0).empty());
}

TEST(FractalTest, GenerateStopsWhenCancelled) {
    const Fractal fractal(100);
    std::stop_source source;
    source.request_stop();

    EXPECT_TRUE(fractal.generate(64, 64, -0.6, 0.0, 3.5, source.get_token()).empty());
}

}  // namespace

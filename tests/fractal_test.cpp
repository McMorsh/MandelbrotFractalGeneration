#include <cmath>
#include <cstddef>
#include <stop_token>
#include <vector>

#include <gtest/gtest.h>

#include "fractal.hpp"
#include "fractal_type.hpp"

namespace {

TEST(FractalTest, PointInsideSetReturnsIterationLimit) {
  const Fractal fractal(200);

  EXPECT_DOUBLE_EQ(fractal.Compute(0.0, 0.0), 200.0);
}

TEST(FractalTest, LeftBulbInside) {
  const Fractal fractal(200);

  EXPECT_DOUBLE_EQ(fractal.Compute(-1.0, 0.0), 200.0);
}

TEST(FractalTest, BoundaryPointStaysInside) {
  const Fractal fractal(200);

  EXPECT_DOUBLE_EQ(fractal.Compute(-2.0, 0.0), 200.0);
}

TEST(FractalTest, PointOutsideEscapesQuickly) {
  const Fractal fractal(200);

  EXPECT_LT(fractal.Compute(3.0, 0.0), 5.0);
}

TEST(FractalTest, SmoothValueIsNotInteger) {
  const Fractal fractal(200);
  const double value = fractal.Compute(3.0, 0.0);

  EXPECT_GT(value, 0.0);
  EXPECT_LT(value, 200.0);
  EXPECT_NE(value, std::floor(value));
}

TEST(FractalTest, GenerateSizeMatchesPixels) {
  const Fractal fractal(100);

  EXPECT_EQ(fractal.Generate(64, 48, -0.6, 0.0, 3.5).size(),
            static_cast<std::size_t>(64 * 48));
}

TEST(FractalTest, GenerateCenterIsInside) {
  const Fractal fractal(100);
  constexpr int kWidth = 64;
  constexpr int kHeight = 64;
  const std::vector<double> data =
      fractal.Generate(kWidth, kHeight, -0.6, 0.0, 3.5);

  ASSERT_EQ(data.size(), static_cast<std::size_t>(kWidth * kHeight));
  const std::size_t center =
      static_cast<std::size_t>(kHeight / 2) * kWidth + kWidth / 2;
  EXPECT_DOUBLE_EQ(data[center], 100.0);
}

TEST(FractalTest, GenerateCornerEscapes) {
  const Fractal fractal(100);
  const std::vector<double> data = fractal.Generate(64, 64, -0.6, 0.0, 3.5);

  ASSERT_FALSE(data.empty());
  EXPECT_LT(data.front(), 100.0);
}

TEST(FractalTest, GenerateIsDeterministic) {
  const Fractal fractal(100);
  const std::vector<double> first = fractal.Generate(32, 24, -0.6, 0.0, 3.5);
  const std::vector<double> second = fractal.Generate(32, 24, -0.6, 0.0, 3.5);

  EXPECT_EQ(first, second);
}

TEST(FractalTest, GenerateRejectsInvalidSize) {
  const Fractal fractal(100);

  EXPECT_TRUE(fractal.Generate(0, 10, -0.6, 0.0, 3.5).empty());
  EXPECT_TRUE(fractal.Generate(10, -1, -0.6, 0.0, 3.5).empty());
}

TEST(FractalTest, GenerateRejectsInvalidSpan) {
  const Fractal fractal(100);

  EXPECT_TRUE(fractal.Generate(10, 10, -0.6, 0.0, 0.0).empty());
  EXPECT_TRUE(fractal.Generate(10, 10, -0.6, 0.0, -1.0).empty());
}

TEST(FractalTest, GenerateStopsWhenCancelled) {
  const Fractal fractal(100);
  std::stop_source source;
  source.request_stop();

  EXPECT_TRUE(
      fractal.Generate(64, 64, -0.6, 0.0, 3.5, source.get_token()).empty());
}

TEST(FractalTest, JuliaWithZeroParameterKeepsUnitDisk) {
  FractalParams params;
  params.type = FractalType::kJulia;
  params.julia_real = 0.0;
  params.julia_imag = 0.0;
  const Fractal fractal(200, params);

  EXPECT_DOUBLE_EQ(fractal.Compute(0.5, 0.0), 200.0);
  EXPECT_LT(fractal.Compute(2.0, 0.0), 5.0);
}

TEST(FractalTest, JuliaDefaultParameterContainsOrigin) {
  FractalParams params;
  params.type = FractalType::kJulia;
  const Fractal fractal(200, params);

  EXPECT_DOUBLE_EQ(fractal.Compute(0.0, 0.0), 200.0);
}

TEST(FractalTest, JuliaGenerateIsDeterministic) {
  FractalParams params;
  params.type = FractalType::kJulia;
  const Fractal fractal(100, params);
  const std::vector<double> first = fractal.Generate(32, 24, 0.0, 0.0, 4.0);
  const std::vector<double> second = fractal.Generate(32, 24, 0.0, 0.0, 4.0);

  EXPECT_EQ(first, second);
}

TEST(FractalTest, BurningShipContainsOrigin) {
  FractalParams params;
  params.type = FractalType::kBurningShip;
  const Fractal fractal(200, params);

  EXPECT_DOUBLE_EQ(fractal.Compute(0.0, 0.0), 200.0);
}

TEST(FractalTest, BurningShipPointOutsideEscapes) {
  FractalParams params;
  params.type = FractalType::kBurningShip;
  const Fractal fractal(200, params);

  EXPECT_LT(fractal.Compute(3.0, 0.0), 5.0);
}

TEST(FractalTest, BurningShipDiffersFromMandelbrot) {
  FractalParams params;
  params.type = FractalType::kBurningShip;
  const Fractal burning_ship(200, params);
  const Fractal mandelbrot(200);

  EXPECT_DOUBLE_EQ(mandelbrot.Compute(-0.5, -0.5), 200.0);
  EXPECT_LT(burning_ship.Compute(-0.5, -0.5), 10.0);
}

TEST(FractalTest, BurningShipGenerateIsDeterministic) {
  FractalParams params;
  params.type = FractalType::kBurningShip;
  const Fractal fractal(100, params);
  const std::vector<double> first = fractal.Generate(32, 24, -0.5, -0.5, 3.5);
  const std::vector<double> second = fractal.Generate(32, 24, -0.5, -0.5, 3.5);

  EXPECT_EQ(first, second);
}

}  // namespace

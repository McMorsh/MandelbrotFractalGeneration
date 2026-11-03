#include <gtest/gtest.h>

#include "fractal_type.hpp"

namespace {

TEST(FractalTypeTest, DefaultsToMandelbrot) {
  EXPECT_EQ(FractalParams{}.type, FractalType::kMandelbrot);
}

TEST(FractalTypeTest, ParsesKnownNames) {
  FractalType type = FractalType::kJulia;

  ASSERT_TRUE(ParseFractalType("mandelbrot", type));
  EXPECT_EQ(type, FractalType::kMandelbrot);
  ASSERT_TRUE(ParseFractalType("julia", type));
  EXPECT_EQ(type, FractalType::kJulia);
  ASSERT_TRUE(ParseFractalType("burningship", type));
  EXPECT_EQ(type, FractalType::kBurningShip);
}

TEST(FractalTypeTest, ParsesNamesCaseInsensitive) {
  FractalType type = FractalType::kMandelbrot;

  ASSERT_TRUE(ParseFractalType("JULIA", type));
  EXPECT_EQ(type, FractalType::kJulia);
  ASSERT_TRUE(ParseFractalType("Burning-Ship", type));
  EXPECT_EQ(type, FractalType::kBurningShip);
}

TEST(FractalTypeTest, RejectsUnknownName) {
  FractalType type = FractalType::kMandelbrot;

  EXPECT_FALSE(ParseFractalType("unknown", type));
  EXPECT_FALSE(ParseFractalType("", type));
  EXPECT_EQ(type, FractalType::kMandelbrot);
}

TEST(FractalTypeTest, NextTypeCycles) {
  EXPECT_EQ(GetNextFractalType(FractalType::kMandelbrot), FractalType::kJulia);
  EXPECT_EQ(GetNextFractalType(FractalType::kJulia), FractalType::kBurningShip);
  EXPECT_EQ(GetNextFractalType(FractalType::kBurningShip),
            FractalType::kMandelbrot);
}

TEST(FractalTypeTest, NamesMatchCli) {
  EXPECT_STREQ(GetFractalTypeName(FractalType::kMandelbrot), "mandelbrot");
  EXPECT_STREQ(GetFractalTypeName(FractalType::kJulia), "julia");
  EXPECT_STREQ(GetFractalTypeName(FractalType::kBurningShip), "burningship");
}

}  // namespace

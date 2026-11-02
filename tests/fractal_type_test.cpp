#include "FractalType.h"

#include <gtest/gtest.h>

namespace {

	TEST(FractalTypeTest, DefaultsToMandelbrot) {
		EXPECT_EQ(FractalParams{}.type, FractalType::Mandelbrot);
	}

	TEST(FractalTypeTest, ParsesKnownNames) {
		FractalType type = FractalType::Julia;

		ASSERT_TRUE(parseFractalType("mandelbrot", type));
		EXPECT_EQ(type, FractalType::Mandelbrot);
		ASSERT_TRUE(parseFractalType("julia", type));
		EXPECT_EQ(type, FractalType::Julia);
		ASSERT_TRUE(parseFractalType("burningship", type));
		EXPECT_EQ(type, FractalType::BurningShip);
	}

	TEST(FractalTypeTest, ParsesNamesCaseInsensitive) {
		FractalType type = FractalType::Mandelbrot;

		ASSERT_TRUE(parseFractalType("JULIA", type));
		EXPECT_EQ(type, FractalType::Julia);
		ASSERT_TRUE(parseFractalType("Burning-Ship", type));
		EXPECT_EQ(type, FractalType::BurningShip);
	}

	TEST(FractalTypeTest, RejectsUnknownName) {
		FractalType type = FractalType::Mandelbrot;

		EXPECT_FALSE(parseFractalType("unknown", type));
		EXPECT_FALSE(parseFractalType("", type));
		EXPECT_EQ(type, FractalType::Mandelbrot);
	}

	TEST(FractalTypeTest, NextTypeCycles) {
		EXPECT_EQ(nextFractalType(FractalType::Mandelbrot), FractalType::Julia);
		EXPECT_EQ(nextFractalType(FractalType::Julia), FractalType::BurningShip);
		EXPECT_EQ(nextFractalType(FractalType::BurningShip), FractalType::Mandelbrot);
	}

	TEST(FractalTypeTest, NamesMatchCli) {
		EXPECT_STREQ(fractalTypeName(FractalType::Mandelbrot), "mandelbrot");
		EXPECT_STREQ(fractalTypeName(FractalType::Julia), "julia");
		EXPECT_STREQ(fractalTypeName(FractalType::BurningShip), "burningship");
	}

} // namespace

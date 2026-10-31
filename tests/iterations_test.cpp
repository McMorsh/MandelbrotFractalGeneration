#include "Iterations.h"

#include "View.h"

#include <gtest/gtest.h>

namespace {

	constexpr double kHomeSpan = 3.5;

	TEST(IterationsTest, HomeSpanUsesBaseIterations) {
		EXPECT_EQ(iterationsForSpan(kHomeSpan), 150);
	}

	TEST(IterationsTest, ViewAppliesBonus) {
		EXPECT_EQ(iterationsForView(kHomeSpan, 100), 250);
	}

	TEST(IterationsTest, ViewClampsToLimits) {
		EXPECT_EQ(iterationsForView(kHomeSpan, -1000), kMinIterations);
		EXPECT_EQ(iterationsForView(kHomeSpan, 5000), kMaxIterations);
	}

	TEST(IterationsTest, StepDownStopsAtFloor) {
		int nextBonus = 0;

		EXPECT_FALSE(nextIterationBonus(kHomeSpan, -100, -kIterationStep, nextBonus));
		EXPECT_EQ(iterationsForView(kHomeSpan, -100), kMinIterations);
	}

	TEST(IterationsTest, StepUpWorksFromFloor) {
		int nextBonus = 0;

		ASSERT_TRUE(nextIterationBonus(kHomeSpan, -100, kIterationStep, nextBonus));
		EXPECT_EQ(nextBonus, 0);
		EXPECT_GT(iterationsForView(kHomeSpan, nextBonus), kMinIterations);
	}

	TEST(IterationsTest, StepUpStopsAtCeiling) {
		int nextBonus = 0;

		EXPECT_FALSE(nextIterationBonus(kMinSpan, 2000, kIterationStep, nextBonus));
		EXPECT_EQ(iterationsForView(kMinSpan, 2000), kMaxIterations);
	}

	TEST(IterationsTest, StepDownWorksFromCeiling) {
		int nextBonus = 0;

		ASSERT_TRUE(nextIterationBonus(kMinSpan, 2000, -kIterationStep, nextBonus));
		EXPECT_EQ(nextBonus, 1900);
		EXPECT_LT(iterationsForView(kMinSpan, nextBonus), kMaxIterations);
	}

	TEST(IterationsTest, ZeroDeltaDoesNothing) {
		int nextBonus = 0;

		EXPECT_FALSE(nextIterationBonus(kHomeSpan, 0, 0, nextBonus));
	}

} // namespace

#include "Iterations.h"

#include "View.h"

#include <gtest/gtest.h>

namespace {

	constexpr double kInitialViewSpan = 3.5;

	TEST(IterationsTest, InitialSpanUsesBaseIterations) {
		EXPECT_EQ(iterationsForSpan(kInitialViewSpan), 150);
	}

	TEST(IterationsTest, ViewAppliesBonus) {
		EXPECT_EQ(iterationsForView(kInitialViewSpan, 100), 250);
	}

	TEST(IterationsTest, ViewClampsToLimits) {
		EXPECT_EQ(iterationsForView(kInitialViewSpan, -1000), kMinIterations);
		EXPECT_EQ(iterationsForView(kInitialViewSpan, 5000), kMaxIterations);
	}

	TEST(IterationsTest, StepDownStopsAtFloor) {
		int nextBonus = 0;

		EXPECT_FALSE(nextIterationBonus(kInitialViewSpan, -100, -kIterationStep, nextBonus));
		EXPECT_EQ(iterationsForView(kInitialViewSpan, -100), kMinIterations);
	}

	TEST(IterationsTest, StepUpWorksFromFloor) {
		int nextBonus = 0;

		ASSERT_TRUE(nextIterationBonus(kInitialViewSpan, -100, kIterationStep, nextBonus));
		EXPECT_EQ(nextBonus, 0);
		EXPECT_GT(iterationsForView(kInitialViewSpan, nextBonus), kMinIterations);
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

		EXPECT_FALSE(nextIterationBonus(kInitialViewSpan, 0, 0, nextBonus));
	}

} // namespace

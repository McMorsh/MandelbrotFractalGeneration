#include <gtest/gtest.h>

#include "iterations.hpp"
#include "view.hpp"

namespace {

constexpr double kInitialViewSpan = 3.5;

TEST(IterationsTest, InitialSpanUsesBaseIterations) {
  EXPECT_EQ(ComputeIterationsForSpan(kInitialViewSpan), 150);
}

TEST(IterationsTest, ViewAppliesBonus) {
  EXPECT_EQ(ComputeIterationsForView(kInitialViewSpan, 100), 250);
}

TEST(IterationsTest, ViewClampsToLimits) {
  EXPECT_EQ(ComputeIterationsForView(kInitialViewSpan, -1000), kMinIterations);
  EXPECT_EQ(ComputeIterationsForView(kInitialViewSpan, 5000), kMaxIterations);
}

TEST(IterationsTest, StepDownStopsAtFloor) {
  int next_bonus = 0;

  EXPECT_FALSE(TryNextIterationBonus(kInitialViewSpan, -100, -kIterationStep,
                                     next_bonus));
  EXPECT_EQ(ComputeIterationsForView(kInitialViewSpan, -100), kMinIterations);
}

TEST(IterationsTest, StepUpWorksFromFloor) {
  int next_bonus = 0;

  ASSERT_TRUE(TryNextIterationBonus(kInitialViewSpan, -100, kIterationStep,
                                    next_bonus));
  EXPECT_EQ(next_bonus, 0);
  EXPECT_GT(ComputeIterationsForView(kInitialViewSpan, next_bonus),
            kMinIterations);
}

TEST(IterationsTest, StepUpStopsAtCeiling) {
  int next_bonus = 0;

  EXPECT_FALSE(
      TryNextIterationBonus(kMinSpan, 2000, kIterationStep, next_bonus));
  EXPECT_EQ(ComputeIterationsForView(kMinSpan, 2000), kMaxIterations);
}

TEST(IterationsTest, StepDownWorksFromCeiling) {
  int next_bonus = 0;

  ASSERT_TRUE(
      TryNextIterationBonus(kMinSpan, 2000, -kIterationStep, next_bonus));
  EXPECT_EQ(next_bonus, 1900);
  EXPECT_LT(ComputeIterationsForView(kMinSpan, next_bonus), kMaxIterations);
}

TEST(IterationsTest, ZeroDeltaDoesNothing) {
  int next_bonus = 0;

  EXPECT_FALSE(TryNextIterationBonus(kInitialViewSpan, 0, 0, next_bonus));
}

}  // namespace

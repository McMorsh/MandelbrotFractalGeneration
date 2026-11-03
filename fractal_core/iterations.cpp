#include "iterations.hpp"

#include <algorithm>
#include <cmath>

#include "view.hpp"

namespace {

constexpr double kBaseIterations = 150.0;
constexpr double kIterationsPerZoomLevel = 100.0;
constexpr double kMaxAutomaticIterations = 3000.0;

}  // namespace

int ComputeIterationsForSpan(double span) {
  const double zoom = kInitialSpan / span;
  const double iterations =
      kBaseIterations +
      kIterationsPerZoomLevel * std::log2(std::max(zoom, 1.0));
  return static_cast<int>(
      std::clamp(iterations, kBaseIterations, kMaxAutomaticIterations));
}

int ComputeIterationsForView(double span, int bonus) {
  return std::clamp(ComputeIterationsForSpan(span) + bonus, kMinIterations,
                    kMaxIterations);
}

bool TryNextIterationBonus(double span, int bonus, int delta, int& next_bonus) {
  if (delta == 0) {
    return false;
  }

  const int candidate =
      std::clamp(bonus + delta, kMinIterationBonus, kMaxIterationBonus);
  if (ComputeIterationsForView(span, candidate) ==
      ComputeIterationsForView(span, bonus)) {
    return false;
  }

  next_bonus = candidate;
  return true;
}

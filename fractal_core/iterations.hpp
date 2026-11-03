#ifndef FRACTALGENERATION_FRACTAL_CORE_ITERATIONS_HPP_
#define FRACTALGENERATION_FRACTAL_CORE_ITERATIONS_HPP_

// Base iteration range and the manual adjustment step.
inline constexpr int kMinIterations = 50;
inline constexpr int kMaxIterations = 5000;
inline constexpr int kIterationStep = 100;

/// Range of the iteration offset (adjustment with the wheel or keys).
inline constexpr int kMinIterationBonus = -2000;
inline constexpr int kMaxIterationBonus = 3000;

/// Chooses the iteration count from the window width in complex units.
int ComputeIterationsForSpan(double span);

/// Adds the manual offset to the automatic count and clamps the result.
int ComputeIterationsForView(double span, int bonus);

/// Checks whether adding delta changes the final iteration count for span.
/// @param bonus current manual offset
/// @param next_bonus receives the new offset
/// @return false at the 50 and 5000 boundaries so accumulation does not move
/// the working point and the reverse step takes effect immediately.
bool TryNextIterationBonus(double span, int bonus, int delta, int& next_bonus);

#endif  // FRACTALGENERATION_FRACTAL_CORE_ITERATIONS_HPP_

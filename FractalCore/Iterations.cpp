#include "Iterations.h"

#include "View.h"

#include <algorithm>
#include <cmath>

int iterationsForSpan(double span) {
	const double zoom = kInitialSpan / span;
	const double iterations = 150.0 + 100.0 * std::log2(std::max(zoom, 1.0));
	return static_cast<int>(std::clamp(iterations, 150.0, 3000.0));
}

int iterationsForView(double span, int bonus) {
	return std::clamp(iterationsForSpan(span) + bonus, kMinIterations, kMaxIterations);
}

bool nextIterationBonus(double span, int bonus, int delta, int& nextBonus) {
	if (delta == 0)
		return false;

	const int candidate = std::clamp(bonus + delta, kMinIterationBonus, kMaxIterationBonus);
	if (iterationsForView(span, candidate) == iterationsForView(span, bonus))
		return false;

	nextBonus = candidate;
	return true;
}

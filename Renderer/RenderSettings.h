#pragma once

#include "FractalType.h"

/// Параметры одного кадра: что и где считать.
struct RenderSettings {
	int width = 1200;
	int height = 800;
	int maxIterations = 0;
	FractalParams fractal;
	double centerX = -0.6;
	double centerY = 0.0;
	double span = 3.5;
};

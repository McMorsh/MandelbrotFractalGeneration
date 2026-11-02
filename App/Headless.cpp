#include "Headless.h"

#include "ColorMap.h"
#include "FractalType.h"
#include "ImageRender.h"
#include "Iterations.h"

#include <cstdio>

int runHeadless(const Options& options) {
	if (!(options.settings.span > 0.0)) {
		std::fprintf(stderr, "Span must be positive.\n");
		return 1;
	}

	RenderSettings settings = options.settings;
	if (settings.maxIterations == 0)
		settings.maxIterations = iterationsForSpan(settings.span);

	if (!isValidRenderSettings(settings)) {
		std::fprintf(stderr, "Invalid render settings. Check size, iterations and span.\n");
		return 1;
	}

	ColorMap colormap;
	if (!colormap.setPalette(options.palette)) {
		std::fprintf(stderr, "Unknown palette: %s (available: Ember, Ocean, Mono)\n", options.palette.c_str());
		return 1;
	}

	std::printf("Rendering %dx%d, %d iterations, %s, palette %s...\n", settings.width, settings.height,
				settings.maxIterations, fractalTypeName(settings.fractal.type), colormap.paletteName());
	std::fflush(stdout);

	if (!saveFractalImage(options.output, settings, colormap)) {
		std::fprintf(stderr, "Could not save %s\n", options.output.c_str());
		return 1;
	}

	std::printf("Saved %s\n", options.output.c_str());
	return 0;
}

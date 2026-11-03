#include "headless.hpp"

#include <cstdio>

#include "color_map.hpp"
#include "fractal_type.hpp"
#include "image_render.hpp"
#include "iterations.hpp"
#include "render_settings.hpp"

int RunHeadless(const Options& options) {
  if (!(options.settings.span > 0.0)) {
    std::fprintf(stderr, "Span must be positive.\n");
    return 1;
  }

  RenderSettings settings = options.settings;
  if (settings.max_iterations == 0) {
    settings.max_iterations = ComputeIterationsForSpan(settings.span);
  }

  if (!IsValidRenderSettings(settings)) {
    std::fprintf(stderr,
                 "Invalid render settings. Check size, iterations and span.\n");
    return 1;
  }

  ColorMap colormap;
  if (!colormap.SetPalette(options.palette)) {
    std::fprintf(stderr,
                 "Unknown palette: %s (available: Ember, Ocean, Mono)\n",
                 options.palette.c_str());
    return 1;
  }

  std::printf("Rendering %dx%d, %d iterations, %s, palette %s...\n",
              settings.width, settings.height, settings.max_iterations,
              GetFractalTypeName(settings.fractal.type),
              colormap.GetPaletteName());
  std::fflush(stdout);

  if (!SaveFractalImage(options.output, settings, colormap)) {
    std::fprintf(stderr, "Could not save %s\n", options.output.c_str());
    return 1;
  }

  std::printf("Saved %s\n", options.output.c_str());
  return 0;
}

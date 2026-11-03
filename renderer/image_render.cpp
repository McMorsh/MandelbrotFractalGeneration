#include "image_render.hpp"

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include <raylib.h>

#include "fractal.hpp"

bool IsValidRenderSettings(const RenderSettings& settings) {
  return settings.width > 0 && settings.width <= kMaxImageDimension &&
         settings.height > 0 && settings.height <= kMaxImageDimension &&
         settings.max_iterations > 0 &&
         settings.max_iterations <= kMaxIterationCount && settings.span > 0.0 &&
         std::isfinite(settings.span) && std::isfinite(settings.center_x) &&
         std::isfinite(settings.center_y) &&
         std::isfinite(settings.fractal.julia_real) &&
         std::isfinite(settings.fractal.julia_imag);
}

bool SaveIterationImage(const std::string& path,
                        const std::vector<double>& data, int width, int height,
                        int max_iterations, const ColorMap& colormap) {
  if (path.empty() || width <= 0 || height <= 0 || max_iterations <= 0) {
    return false;
  }
  if (width > kMaxImageDimension || height > kMaxImageDimension ||
      max_iterations > kMaxIterationCount) {
    return false;
  }
  if (data.size() != static_cast<std::size_t>(width) * height) {
    return false;
  }

  Image image = GenImageColor(width, height, BLACK);
  if (image.data == nullptr) {
    return false;
  }

  auto* pixels = static_cast<Color*>(image.data);

#pragma omp parallel for schedule(static)
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      const std::size_t idx = static_cast<std::size_t>(y) * width + x;
      pixels[idx] = colormap.GetColor(data[idx], max_iterations);
    }
  }

  const bool saved = ExportImage(image, path.c_str());
  UnloadImage(image);
  return saved;
}

bool SaveFractalImage(const std::string& path, const RenderSettings& settings,
                      const ColorMap& colormap) {
  if (path.empty() || !IsValidRenderSettings(settings)) {
    return false;
  }

  // Headless mode does not need raylib INFO messages.
  SetTraceLogLevel(LOG_WARNING);

  const Fractal fractal(settings.max_iterations, settings.fractal);
  const std::vector<double> data =
      fractal.Generate(settings.width, settings.height, settings.center_x,
                       settings.center_y, settings.span);
  if (data.empty()) {
    return false;
  }

  return SaveIterationImage(path, data, settings.width, settings.height,
                            settings.max_iterations, colormap);
}

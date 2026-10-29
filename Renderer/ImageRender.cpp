#include "ImageRender.h"
#include "Fractal.h"

#include <cmath>
#include <cstddef>
#include <vector>

bool isValidRenderSettings(const RenderSettings& settings) {
    return settings.width > 0 && settings.width <= 8192 &&
           settings.height > 0 && settings.height <= 8192 &&
           settings.maxIterations > 0 && settings.maxIterations <= 100000 &&
           settings.span > 0.0 && std::isfinite(settings.span) &&
           std::isfinite(settings.centerX) && std::isfinite(settings.centerY);
}

bool saveFractalImage(const std::string& path, const RenderSettings& settings,
                      const ColorMap& colormap) {
    if (path.empty() || !isValidRenderSettings(settings)) return false;

    // В headless-режиме не нужны служебные INFO-сообщения raylib
    SetTraceLogLevel(LOG_WARNING);

    const Fractal fractal(settings.maxIterations);
    const std::vector<double> data = fractal.generate(
        settings.width, settings.height, settings.centerX, settings.centerY, settings.span);
    if (data.empty()) return false;

    Image image = GenImageColor(settings.width, settings.height, BLACK);
    auto* pixels = static_cast<Color*>(image.data);

#pragma omp parallel for schedule(static)
    for (int y = 0; y < settings.height; y++) {
        for (int x = 0; x < settings.width; x++) {
            const std::size_t idx = static_cast<std::size_t>(y) * settings.width + x;
            pixels[idx] = colormap.getColor(data[idx], settings.maxIterations);
        }
    }

    const bool saved = ExportImage(image, path.c_str());
    UnloadImage(image);
    return saved;
}

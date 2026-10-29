#ifndef MANDELBROT_IMAGE_RENDER_H
#define MANDELBROT_IMAGE_RENDER_H

#pragma once
#include "ColorMap.h"
#include <string>

// Параметры кадра для рендера без окна.
struct RenderSettings {
    int width = 1200;
    int height = 800;
    int maxIterations = 0;
    double centerX = -0.6;
    double centerY = 0.0;
    double span = 3.5;
};

// Проверяет размеры, число итераций и границы окна.
bool isValidRenderSettings(const RenderSettings& settings);

// Рендерит фрактал и сохраняет его в PNG.
// Возвращает false, если параметры некорректны или файл не записан.
bool saveFractalImage(const std::string& path, const RenderSettings& settings,
                      const ColorMap& colormap);

#endif //MANDELBROT_IMAGE_RENDER_H

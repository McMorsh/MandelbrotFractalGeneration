#pragma once

#include "ColorMap.h"
#include "RenderSettings.h"
#include <string>
#include <vector>

/// Максимальный размер стороны изображения в пикселях.
inline constexpr int kMaxImageDimension = 8192;

/// Проверяет размеры, число итераций, span, центр и параметры фрактала.
bool isValidRenderSettings(const RenderSettings& settings);

/// Сохраняет готовую карту итераций в PNG без окна.
/// @param data карта размера width * height
/// @return false, если размеры не совпадают с data или файл не записан.
bool saveIterationImage(const std::string& path, const std::vector<double>& data, int width, int height,
						int maxIterations, const ColorMap& colormap);

/// Рендерит фрактал и сохраняет его в PNG.
/// @return false, если параметры некорректны или файл не записан.
bool saveFractalImage(const std::string& path, const RenderSettings& settings, const ColorMap& colormap);

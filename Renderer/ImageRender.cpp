#include "ImageRender.h"
#include "Fractal.h"

#include <cmath>
#include <cstddef>
#include <vector>

bool isValidRenderSettings(const RenderSettings& settings) {
	return settings.width > 0 && settings.width <= kMaxImageDimension && settings.height > 0 &&
		   settings.height <= kMaxImageDimension && settings.maxIterations > 0 && settings.maxIterations <= 100000 &&
		   settings.span > 0.0 && std::isfinite(settings.span) && std::isfinite(settings.centerX) &&
		   std::isfinite(settings.centerY);
}

bool saveIterationImage(const std::string& path, const std::vector<double>& data, int width, int height,
						int maxIterations, const ColorMap& colormap) {
	if (path.empty() || width <= 0 || height <= 0 || maxIterations <= 0)
		return false;
	if (width > kMaxImageDimension || height > kMaxImageDimension || maxIterations > 100000)
		return false;
	if (data.size() != static_cast<std::size_t>(width) * height)
		return false;

	Image image = GenImageColor(width, height, BLACK);
	if (image.data == nullptr)
		return false;

	auto* pixels = static_cast<Color*>(image.data);

#pragma omp parallel for schedule(static)
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			const std::size_t idx = static_cast<std::size_t>(y) * width + x;
			pixels[idx] = colormap.getColor(data[idx], maxIterations);
		}
	}

	const bool saved = ExportImage(image, path.c_str());
	UnloadImage(image);
	return saved;
}

bool saveFractalImage(const std::string& path, const RenderSettings& settings, const ColorMap& colormap) {
	if (path.empty() || !isValidRenderSettings(settings))
		return false;

	// В headless-режиме не нужны служебные INFO-сообщения raylib
	SetTraceLogLevel(LOG_WARNING);

	const Fractal fractal(settings.maxIterations);
	const std::vector<double> data =
		fractal.generate(settings.width, settings.height, settings.centerX, settings.centerY, settings.span);
	if (data.empty())
		return false;

	return saveIterationImage(path, data, settings.width, settings.height, settings.maxIterations, colormap);
}

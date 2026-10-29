#include "Fractal.h"

#include <cmath>
#include <complex>
#include <cstddef>

Fractal::Fractal(int maxIterations) : maxIterations(maxIterations) {
}

double Fractal::compute(double real, double imag) const {
	const std::complex<double> c(real, imag);
	std::complex<double> z(0.0, 0.0);
	int iterations = 0;

	// std::norm(z) возвращает |z|^2, поэтому корень не нужен
	while (std::norm(z) <= 4.0 && iterations < maxIterations) {
		z = z * z + c;
		iterations++;
	}

	// Точка не успела выйти за предел, считаем её принадлежащей множеству
	if (iterations >= maxIterations) {
		return static_cast<double>(maxIterations);
	}

	// Непрерывная оценка убирает ступени между цветовыми полосами.
	// 0.5 * log2(|z|^2) = log2(|z|), поэтому nu = log2(log2(|z|)).
	const double nu = std::log2(0.5 * std::log2(std::norm(z)));
	return iterations + 1.0 - nu;
}

std::vector<double> Fractal::generate(int width, int height, double centerX, double centerY, double span,
									  std::stop_token stopToken) const {
	if (width <= 0 || height <= 0 || span <= 0.0)
		return {};
	if (stopToken.stop_requested())
		return {};

	std::vector<double> result(static_cast<std::size_t>(width) * height);

	// Масштаб одинаков по обеим осям, чтобы фрактал не растягивался
	const double scale = span / width;
	const double halfWidth = width * 0.5;
	const double halfHeight = height * 0.5;

	// Строки независимы, поэтому распараллеливаем внешний цикл
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < height; y++) {
		if (stopToken.stop_requested())
			continue;

		const double imag = centerY - (y - halfHeight) * scale;
		for (int x = 0; x < width; x++) {
			const double real = centerX + (x - halfWidth) * scale;
			result[static_cast<std::size_t>(y) * width + x] = compute(real, imag);
		}
	}

	if (stopToken.stop_requested())
		return {};

	return result;
}

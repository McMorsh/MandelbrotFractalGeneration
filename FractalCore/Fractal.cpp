#include "Fractal.h"

#include <cmath>
#include <complex>
#include <cstddef>

Fractal::Fractal(int maxIterations) : maxIterations(maxIterations) {
}

Fractal::Fractal(int maxIterations, const FractalParams& params) : maxIterations(maxIterations), params(params) {
}

double Fractal::compute(double real, double imag) const {
	// У Мандельброта и Burning Ship c - точка, z0 = 0; у Julia наоборот.
	std::complex<double> z;
	std::complex<double> c;
	if (params.type == FractalType::Julia) {
		z = {real, imag};
		c = {params.juliaReal, params.juliaImag};
	} else {
		z = {0.0, 0.0};
		// У Burning Ship мнимая часть c инвертируется для классической ориентации
		c = {real, params.type == FractalType::BurningShip ? -imag : imag};
	}
	int iterations = 0;

	if (params.type == FractalType::BurningShip) {
		// Формула: z = (|Re z| + i|Im z|)^2 + c
		while (std::norm(z) <= 4.0 && iterations < maxIterations) {
			z = {std::abs(z.real()), std::abs(z.imag())};
			z = z * z + c;
			iterations++;
		}
	} else {
		// std::norm(z) возвращает |z|^2, поэтому корень не нужен
		while (std::norm(z) <= 4.0 && iterations < maxIterations) {
			z = z * z + c;
			iterations++;
		}
	}

	// Точка не успела выйти за предел, считаем её принадлежащей множеству
	if (iterations >= maxIterations) {
		return static_cast<double>(maxIterations);
	}

	// Непрерывная оценка убирает ступени: nu = log2(log2(|z|)).
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
			// Отмена проверяется редко, чтобы не тормозить внутренний цикл
			if ((x & 63) == 0 && stopToken.stop_requested())
				break;

			const double real = centerX + (x - halfWidth) * scale;
			result[static_cast<std::size_t>(y) * width + x] = compute(real, imag);
		}
	}

	if (stopToken.stop_requested())
		return {};

	return result;
}

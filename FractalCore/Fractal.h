#pragma once

#include "FractalType.h"

#include <stop_token>
#include <vector>

/// Считает выбранный фрактал в заданном окне комплексной плоскости.
class Fractal {
  private:
	int maxIterations = 0;
	FractalParams params;

  public:
	/// Создаёт расчёт множества Мандельброта с заданным числом итераций.
	explicit Fractal(int maxIterations);
	/// Создаёт расчёт выбранного фрактала.
	Fractal(int maxIterations, const FractalParams& params);

	/// Сглаженное число итераций для точки real + i*imag.
	/// У Мандельброта и Burning Ship z0 = 0, а точка задаёт c; у Julia наоборот.
	/// @return maxIterations, если точка не вышла за |z| = 2.
	double compute(double real, double imag) const;

	/// Возвращает карту сглаженных итераций для окна width x height.
	/// @param span ширина окна в комплексных единицах
	/// @param stopToken если он остановлен, функция возвращает пустую карту
	/// @return width * height значений в построчном порядке; ось Im направлена вверх.
	/// Пиксель (x, y) лежит по индексу y * width + x.
	std::vector<double> generate(int width, int height, double centerX, double centerY, double span,
								 std::stop_token stopToken = {}) const;
};

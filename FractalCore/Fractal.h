#pragma once

#include "FractalType.h"

#include <stop_token>
#include <vector>

// Вычисляет выбранный фрактал для заданного окна комплексной плоскости.
class Fractal {
  private:
	int maxIterations = 0;
	FractalParams params;

  public:
	explicit Fractal(int maxIterations);
	Fractal(int maxIterations, const FractalParams& params);

	// Сглаженное число итераций для точки real + i*imag.
	// У Мандельброта это параметр c при z0 = 0, у Julia это z0 при постоянном c из params.
	// Если точка так и не вышла за предел |z| = 2, возвращает maxIterations.
	double compute(double real, double imag) const;

	// Карта сглаженных итераций для окна width x height.
	// centerX и centerY задают центр окна, span задаёт его ширину
	// в комплексных единицах. Ось Im направлена вверх.
	// Если stopToken остановлен, возвращает пустой вектор.
	std::vector<double> generate(int width, int height, double centerX, double centerY, double span,
								 std::stop_token stopToken = {}) const;
};

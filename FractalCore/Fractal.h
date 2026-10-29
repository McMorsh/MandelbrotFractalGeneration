#pragma once

#include <stop_token>
#include <vector>

// Вычисляет множество Мандельброта для заданного окна комплексной плоскости.
class Fractal {
private:
    int maxIterations = 0;
public:
    explicit Fractal(int maxIterations);

    // Сглаженное число итераций для точки c = real + i*imag.
    // Если точка так и не вышла за предел |z| = 2, возвращает maxIterations.
    double compute(double real, double imag) const;

    // Карта сглаженных итераций для окна width x height.
    // centerX и centerY задают центр окна, span задаёт его ширину
    // в комплексных единицах. Ось Im направлена вверх.
    // Если stopToken остановлен, возвращает пустой вектор.
    std::vector<double> generate(int width, int height, double centerX,
                                 double centerY, double span,
                                 std::stop_token stopToken = {}) const;
};

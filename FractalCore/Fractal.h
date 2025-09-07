#ifndef FARKTALGENERATION_FRACTAL_H
#define FARKTALGENERATION_FRACTAL_H

#pragma once
#include <vector>


// Класс для вычисления фрактального множества Мандельброта.
// Хранит максимальное число итераций
class Fractal {
private:
    int maxIterations = 0;
public:
    Fractal(int maxIterations);
    // Вычисление чисто итераций для одной точки
    int compute(double real, double imag) const;
    // Генерация массива итерация для изображения
    std::vector<int> generate(int width, int height) const;
};


#endif //FARKTALGENERATION_FRACTAL_H
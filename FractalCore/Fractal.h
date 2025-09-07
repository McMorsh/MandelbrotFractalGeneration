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
    // Конструктор
    Fractal(int maxIterations);
    // Вычисление чисто итераций для одной точки
    // real - действительная часть координаты
    // imag - мнимая часть координаты
    // Возвращает количество итераций до выхода за предел модуля 2
    int compute(double real, double imag) const;
    // Генерация массива итерация для изображения
    // width - ширина изображения в пикселях
    // height - высота изображения в пикселях
    // Возвращает вектор целых чисел, где каждый элемент — количество итераций для соответствующего пикселя

    std::vector<int> generate(int width, int height) const;
};


#endif //FARKTALGENERATION_FRACTAL_H
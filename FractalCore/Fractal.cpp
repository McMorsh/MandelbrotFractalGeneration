#include "Fractal.h"
#include <complex>
#include <iostream>
#include <omp.h>

// Конструктор
Fractal::Fractal(int maxIterations) : maxIterations(maxIterations) {}

// Метод compute вычисляет количество итераций для точки на комплексной плоскости
int Fractal::compute(double real, double imag) const {
    // комплексное число complex = real + i*imag
    std::complex<double> complex(real, imag);
    std::complex<double> z(0, 0);
    int iterations = 0;

    // Основной цикл
    while (abs(z) <= 2.0 && iterations < maxIterations) {
        z = z * z + complex;
        iterations++;
    }

    return iterations;
};

// Метод generate создает полный массив итераций для всего изображения
std::vector<int> Fractal::generate(int width, int height) const {
    std::vector<int> result(width * height);

    // Используем OpenMP для параллельной обработки строк изображения
#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Преобразуем координаты пикселя в координаты комплексной плоскости
            double real = (x - width / 2.0) * 4.0 / width;
            double imag = (y - height / 2.0) * 4.0 / height;
            result[y * width + x] = compute(real, imag);

            // int thread_num = omp_get_thread_num();
            // std::cout << "Thread " << thread_num << " computing pixel (" << x << "," << y << ")\n";
        }
    }

    return result;
}


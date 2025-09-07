#include "Fractal.h"
#include <complex>
#include <iostream>
#include <omp.h>

// Конструктор
Fractal::Fractal(int maxIterations) : maxIterations(maxIterations) {}

//
int Fractal::compute(double real, double imag) const {
    std::complex<double> complex(real, imag);
    std::complex<double> z(0, 0);
    int iterations = 0;

    while (abs(z) <= 2.0 && iterations < maxIterations) {
        z = z * z + complex;
        iterations++;
    }

    return iterations;
};

std::vector<int> Fractal::generate(int width, int height) const {
    std::vector<int> result(width * height);


#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double real = (x - width / 2.0) * 4.0 / width;
            double imag = (y - height / 2.0) * 4.0 / height;
            result[y * width + x] = compute(real, imag);

            // int thread_num = omp_get_thread_num();
            // std::cout << "Thread " << thread_num << " computing pixel (" << x << "," << y << ")\n";
        }
    }

    return result;
}


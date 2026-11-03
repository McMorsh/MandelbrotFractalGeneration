#include "fractal.hpp"

#include <cmath>
#include <complex>
#include <cstddef>
#include <stop_token>
#include <vector>

#include "fractal_type.hpp"

Fractal::Fractal(int max_iterations) : max_iterations_(max_iterations) {}

Fractal::Fractal(int max_iterations, const FractalParams& params)
    : max_iterations_(max_iterations), params_(params) {}

double Fractal::Compute(double real, double imag) const {
  // For Mandelbrot and Burning Ship c is the point and z0 = 0; for Julia it
  // is the other way around.
  std::complex<double> z;
  std::complex<double> c;
  if (params_.type == FractalType::kJulia) {
    z = {real, imag};
    c = {params_.julia_real, params_.julia_imag};
  } else {
    z = {0.0, 0.0};
    // Burning Ship inverts the imaginary part of c for the classic
    // orientation.
    c = {real, params_.type == FractalType::kBurningShip ? -imag : imag};
  }
  int iterations = 0;

  if (params_.type == FractalType::kBurningShip) {
    // Formula: z = (|Re z| + i|Im z|)^2 + c.
    while (std::norm(z) <= 4.0 && iterations < max_iterations_) {
      z = {std::abs(z.real()), std::abs(z.imag())};
      z = z * z + c;
      iterations++;
    }
  } else {
    // std::norm(z) returns |z|^2, so the square root is unnecessary.
    while (std::norm(z) <= 4.0 && iterations < max_iterations_) {
      z = z * z + c;
      iterations++;
    }
  }

  // The point never escaped, so it belongs to the set.
  if (iterations >= max_iterations_) {
    return static_cast<double>(max_iterations_);
  }

  // Continuous estimate removes banding: nu = log2(log2(|z|)).
  const double nu = std::log2(0.5 * std::log2(std::norm(z)));
  return iterations + 1.0 - nu;
}

std::vector<double> Fractal::Generate(int width, int height, double center_x,
                                      double center_y, double span,
                                      std::stop_token stop_token) const {
  if (width <= 0 || height <= 0 || span <= 0.0) {
    return {};
  }
  if (stop_token.stop_requested()) {
    return {};
  }

  std::vector<double> result(static_cast<std::size_t>(width) * height);

  // The scale is the same on both axes so the fractal is not stretched.
  const double scale = span / width;
  const double half_width = width * 0.5;
  const double half_height = height * 0.5;

  // Rows are independent, so the outer loop is parallelized.
#pragma omp parallel for schedule(dynamic)
  for (int y = 0; y < height; y++) {
    if (stop_token.stop_requested()) {
      continue;
    }

    const double imag = center_y - (y - half_height) * scale;
    for (int x = 0; x < width; x++) {
      // Cancellation is checked rarely to keep the inner loop fast.
      if ((x & 63) == 0 && stop_token.stop_requested()) {
        break;
      }

      const double real = center_x + (x - half_width) * scale;
      result[static_cast<std::size_t>(y) * width + x] = Compute(real, imag);
    }
  }

  if (stop_token.stop_requested()) {
    return {};
  }

  return result;
}

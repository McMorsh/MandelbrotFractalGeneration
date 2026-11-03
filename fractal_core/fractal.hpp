#ifndef FRACTALGENERATION_FRACTAL_CORE_FRACTAL_HPP_
#define FRACTALGENERATION_FRACTAL_CORE_FRACTAL_HPP_

#include <stop_token>
#include <vector>

#include "fractal_type.hpp"

/// Computes the selected fractal for a window of the complex plane.
class Fractal {
 public:
  /// Creates a Mandelbrot set computation with the given iteration limit.
  explicit Fractal(int max_iterations);
  /// Creates a computation for the selected fractal.
  Fractal(int max_iterations, const FractalParams& params);

  /// Smoothed iteration count for the point real + i * imag.
  /// For Mandelbrot and Burning Ship z0 = 0 and the point defines c; for
  /// Julia it is the other way around.
  /// @return max_iterations if the point did not escape |z| = 2.
  double Compute(double real, double imag) const;

  /// Returns a map of smoothed iteration counts for a width x height window.
  /// @param span window width in complex units
  /// @param stop_token if stopped, the function returns an empty map
  /// @return width * height values in row-major order; the Im axis points up.
  /// The pixel (x, y) is stored at index y * width + x.
  std::vector<double> Generate(int width, int height, double center_x,
                               double center_y, double span,
                               std::stop_token stop_token = {}) const;

 private:
  int max_iterations_ = 0;
  FractalParams params_;
};

#endif  // FRACTALGENERATION_FRACTAL_CORE_FRACTAL_HPP_

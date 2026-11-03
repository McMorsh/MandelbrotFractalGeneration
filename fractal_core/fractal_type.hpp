#ifndef FRACTALGENERATION_FRACTAL_CORE_FRACTAL_TYPE_HPP_
#define FRACTALGENERATION_FRACTAL_CORE_FRACTAL_TYPE_HPP_

#include <string>

/// Fractal type.
enum class FractalType {
  kMandelbrot,
  kJulia,
  kBurningShip,
};

/// Fractal parameters: the fractal type and the Julia parameter c.
struct FractalParams {
  FractalType type = FractalType::kMandelbrot;
  double julia_real = -0.8;
  double julia_imag = 0.156;
};

/// Returns the next fractal type in a cycle.
FractalType GetNextFractalType(FractalType type);

/// Parses a fractal type name case-insensitively: mandelbrot, julia,
/// burningship, or burning-ship.
/// @param type receives the parsed type
/// @return false if the name is not recognized.
bool ParseFractalType(const std::string& text, FractalType& type);

/// Returns the command-line name of the type: mandelbrot, julia, or
/// burningship.
const char* GetFractalTypeName(FractalType type);

#endif  // FRACTALGENERATION_FRACTAL_CORE_FRACTAL_TYPE_HPP_

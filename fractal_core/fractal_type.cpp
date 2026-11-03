#include "fractal_type.hpp"

#include <cctype>
#include <string>

FractalType GetNextFractalType(FractalType type) {
  switch (type) {
    case FractalType::kMandelbrot:
      return FractalType::kJulia;
    case FractalType::kJulia:
      return FractalType::kBurningShip;
    case FractalType::kBurningShip:
      return FractalType::kMandelbrot;
  }
  return FractalType::kMandelbrot;
}

bool ParseFractalType(const std::string& text, FractalType& type) {
  std::string lower;
  lower.reserve(text.size());
  for (const char ch : text) {
    lower.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
  }

  if (lower == "mandelbrot") {
    type = FractalType::kMandelbrot;
    return true;
  }
  if (lower == "julia") {
    type = FractalType::kJulia;
    return true;
  }
  if (lower == "burningship" || lower == "burning-ship") {
    type = FractalType::kBurningShip;
    return true;
  }

  return false;
}

const char* GetFractalTypeName(FractalType type) {
  switch (type) {
    case FractalType::kMandelbrot:
      return "mandelbrot";
    case FractalType::kJulia:
      return "julia";
    case FractalType::kBurningShip:
      return "burningship";
  }
  return "mandelbrot";
}

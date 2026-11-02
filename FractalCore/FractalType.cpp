#include "FractalType.h"

#include <cctype>

FractalType nextFractalType(FractalType type) {
	return type == FractalType::Mandelbrot ? FractalType::Julia : FractalType::Mandelbrot;
}

bool parseFractalType(const std::string& text, FractalType& type) {
	std::string lower;
	lower.reserve(text.size());
	for (const char ch : text)
		lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));

	if (lower == "mandelbrot") {
		type = FractalType::Mandelbrot;
		return true;
	}
	if (lower == "julia") {
		type = FractalType::Julia;
		return true;
	}

	return false;
}

const char* fractalTypeName(FractalType type) {
	return type == FractalType::Julia ? "julia" : "mandelbrot";
}

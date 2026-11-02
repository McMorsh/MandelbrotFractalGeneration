#include "FractalType.h"

#include <cctype>

FractalType nextFractalType(FractalType type) {
	switch (type) {
		case FractalType::Mandelbrot:
			return FractalType::Julia;
		case FractalType::Julia:
			return FractalType::BurningShip;
		case FractalType::BurningShip:
			return FractalType::Mandelbrot;
	}
	return FractalType::Mandelbrot;
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
	if (lower == "burningship" || lower == "burning-ship") {
		type = FractalType::BurningShip;
		return true;
	}

	return false;
}

const char* fractalTypeName(FractalType type) {
	switch (type) {
		case FractalType::Mandelbrot:
			return "mandelbrot";
		case FractalType::Julia:
			return "julia";
		case FractalType::BurningShip:
			return "burningship";
	}
	return "mandelbrot";
}

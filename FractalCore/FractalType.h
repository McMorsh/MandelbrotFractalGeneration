#pragma once

#include <string>

/// Тип фрактала.
enum class FractalType {
	Mandelbrot,
	Julia,
	BurningShip,
};

/// Параметры расчёта: тип фрактала и параметр "c" для Julia.
struct FractalParams {
	FractalType type = FractalType::Mandelbrot;
	double juliaReal = -0.8;
	double juliaImag = 0.156;
};

/// Возвращает следующий тип фрактала по кругу.
FractalType nextFractalType(FractalType type);

/// Распознаёт имя типа без учёта регистра: mandelbrot, julia, burningship или burning-ship.
/// @param type сюда записывается распознанный тип
/// @return false, если имя не распознано.
bool parseFractalType(const std::string& text, FractalType& type);

/// Имя типа для командной строки: mandelbrot, julia или burningship.
const char* fractalTypeName(FractalType type);

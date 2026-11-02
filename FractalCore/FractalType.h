#pragma once

#include <string>

// Вариант фрактала.
enum class FractalType {
	Mandelbrot,
	Julia,
};

// Параметры расчёта: тип и постоянный c для Julia.
struct FractalParams {
	FractalType type = FractalType::Mandelbrot;
	double juliaReal = -0.8;
	double juliaImag = 0.156;
};

// Возвращает следующий тип по кругу.
FractalType nextFractalType(FractalType type);

// Разбирает имя типа без учёта регистра.
// Возвращает false, если имя неизвестно.
bool parseFractalType(const std::string& text, FractalType& type);

// Имя типа для командной строки.
const char* fractalTypeName(FractalType type);

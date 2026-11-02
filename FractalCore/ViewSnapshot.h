#pragma once

#include "View.h"

#include <string>

/// Параметры вида, сохранённые в буфер обмена или заметку.
struct ViewSnapshot {
	double centerX = kInitialCenterX;
	double centerY = kInitialCenterY;
	double span = kInitialSpan;
	int iterations = 0;
	std::string palette;
};

/// Форматирует центр, span, зум, итерации и палитру в текст.
/// Центр и span печатаются с max_digits10 значащими цифрами, чтобы восстановить их без потерь;
/// зум печатается для чтения.
std::string formatViewSnapshot(const View& view, int iterations, const std::string& palette);

/// Читает текст, созданный formatViewSnapshot.
/// Незнакомые строки и строку зума пропускает, span проверяет по диапазону View.
/// @param snapshot сюда записывается результат
/// @return false, если обязательных полей нет или значения некорректны.
bool parseViewSnapshot(const std::string& text, ViewSnapshot& snapshot);

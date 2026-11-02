#pragma once

#include <cstddef>
#include <raylib.h>
#include <string>
#include <vector>

/// По сглаженному числу итераций выбирает цвет пикселя.
class ColorMap {
  private:
	std::size_t paletteIndex = 0;
	std::vector<Color> colors;

	static Color lerp(Color from, Color to, float t);

  public:
	ColorMap();

	/// Переключает палитру на следующую по кругу.
	void nextPalette();

	/// Выбирает палитру по имени без учёта регистра: Ember, Ocean или Mono.
	/// @return false, если такого имени нет.
	bool setPalette(const std::string& name);

	/// Имя текущей палитры: Ember, Ocean или Mono.
	const char* paletteName() const;

	/// Цвет пикселя по сглаженному числу итераций.
	/// Точки с iter >= maxIter считаются внутренними и окрашиваются в чёрный.
	Color getColor(double iter, int maxIter) const;
};

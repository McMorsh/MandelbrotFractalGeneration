#ifndef MANDELBROT_COLORMAP_H
#define MANDELBROT_COLORMAP_H

#pragma once
#include <raylib.h>
#include <cstddef>
#include <vector>

// Преобразует сглаженное число итераций в цвет.
class ColorMap {
private:
    std::size_t paletteIndex = 0;
    std::vector<Color> colors;

    static Color lerp(Color from, Color to, float t);
public:
    ColorMap();

    // Переключает палитру на следующую.
    void nextPalette();

    // Имя текущей палитры.
    const char* paletteName() const;

    // iter: сглаженное число итераций.
    // maxIter: порог, начиная с которого точка считается внутренней.
    Color getColor(double iter, int maxIter) const;
};

#endif //MANDELBROT_COLORMAP_H

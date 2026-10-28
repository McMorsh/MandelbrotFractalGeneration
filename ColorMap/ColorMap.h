#ifndef MANDELBROT_COLORMAP_H
#define MANDELBROT_COLORMAP_H

#include <raylib.h>
#include <vector>

// Преобразует сглаженное число итераций в цвет.
class ColorMap {
private:
    std::vector<Color> colors;

    static Color lerp(Color from, Color to, float t);
public:
    ColorMap();

    // iter: сглаженное число итераций.
    // maxIter: порог, начиная с которого точка считается внутренней.
    Color getColor(double iter, int maxIter) const;
};

#endif //MANDELBROT_COLORMAP_H

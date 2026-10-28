#include "ColorMap.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

ColorMap::ColorMap() {
    // Цвета идут от тёмного фона к яркой границе множества.
    // Крайние цвета тёмные, поэтому при зацикливании нет резкого шва.
    colors = {
        {  5,   3,  20, 255},
        { 25,  15,  70, 255},
        { 80,  35, 140, 255},
        {160,  50, 150, 255},
        {235, 100, 100, 255},
        {255, 180,  80, 255},
        {255, 245, 210, 255},
        {180,  60, 140, 255},
        {100,  40, 150, 255},
        { 40,  25, 100, 255},
    };
}

Color ColorMap::lerp(Color from, Color to, float t) {
    return {
        static_cast<unsigned char>(from.r + (to.r - from.r) * t),
        static_cast<unsigned char>(from.g + (to.g - from.g) * t),
        static_cast<unsigned char>(from.b + (to.b - from.b) * t),
        255
    };
}

Color ColorMap::getColor(double iter, int maxIter) const {
    if (maxIter <= 0 || iter >= static_cast<double>(maxIter)) return BLACK;

    // С ростом maxIter цикл удлиняется, иначе на глубоком зуме
    // цветовые полосы становятся слишком частыми.
    const double cycle = std::max(20.0, static_cast<double>(maxIter) / 25.0);

    const double t = std::max(0.0, iter) / cycle;
    const double wrapped = t - std::floor(t);
    const double position = wrapped * static_cast<double>(colors.size());

    const auto index = static_cast<std::size_t>(position);
    const std::size_t next = (index + 1) % colors.size();
    const float frac = static_cast<float>(position - static_cast<double>(index));

    return lerp(colors[index], colors[next], frac);
}

#include "ColorMap.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>

namespace {
using Palette = std::vector<Color>;

// Замкнутые палитры: последний цвет близок к первому,
// поэтому при зацикливании нет видимого шва.
const std::vector<Palette>& palettes() {
    static const std::vector<Palette> list = {
        {
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
        },
        {
            {  2,   6,  20, 255},
            {  4,  26,  64, 255},
            {  8,  70, 120, 255},
            { 16, 130, 170, 255},
            { 45, 190, 200, 255},
            {140, 235, 235, 255},
            {235, 255, 250, 255},
            {120, 210, 230, 255},
            { 45, 130, 190, 255},
            { 10,  55, 115, 255},
        },
        {
            { 12,  12,  16, 255},
            { 35,  35,  42, 255},
            { 70,  70,  80, 255},
            {115, 115, 125, 255},
            {165, 165, 175, 255},
            {215, 215, 225, 255},
            {255, 255, 255, 255},
            {190, 190, 200, 255},
            {120, 120, 130, 255},
            { 50,  50,  58, 255},
        },
    };
    return list;
}

const char* const kPaletteNames[] = {"Ember", "Ocean", "Mono"};

bool sameName(const std::string& value, const char* name) {
    std::size_t i = 0;
    for (; i < value.size() && name[i] != '\0'; ++i) {
        const auto a = static_cast<unsigned char>(value[i]);
        const auto b = static_cast<unsigned char>(name[i]);
        if (std::tolower(a) != std::tolower(b)) return false;
    }
    return i == value.size() && name[i] == '\0';
}
}

ColorMap::ColorMap() : colors(palettes().front()) {}

void ColorMap::nextPalette() {
    paletteIndex = (paletteIndex + 1) % palettes().size();
    colors = palettes()[paletteIndex];
}

bool ColorMap::setPalette(const std::string& name) {
    for (std::size_t i = 0; i < palettes().size(); ++i) {
        if (sameName(name, kPaletteNames[i])) {
            paletteIndex = i;
            colors = palettes()[i];
            return true;
        }
    }
    return false;
}

const char* ColorMap::paletteName() const {
    return kPaletteNames[paletteIndex];
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
    if (colors.empty() || maxIter <= 0) return BLACK;
    if (iter >= static_cast<double>(maxIter)) return BLACK;

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

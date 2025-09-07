#include "ColorMap.h"

ColorMap::ColorMap() {
    colors.push_back(BLACK);
    colors.push_back(DARKGRAY);
    colors.push_back(GRAY);
    colors.push_back(LIGHTGRAY);
    colors.push_back(WHITE);
    colors.push_back(RED);
    colors.push_back(GREEN);
    colors.push_back(BLUE);
    colors.push_back(YELLOW);
    colors.push_back(MAGENTA);
}

Color ColorMap::getColor(int iter, int max_iter) const {
    if (colors.empty()) return BLACK;
    if (iter >= max_iter) return BLACK;

    float t = static_cast<float>(iter) / max_iter;
    int idx1 = static_cast<int>(t * (colors.size() - 1));
    int idx2 = (idx1 + 1) % colors.size();
    float frac = t * (colors.size() - 1) - idx1;

    Color c1 = colors[idx1];
    Color c2 = colors[idx2];
    return {
        static_cast<unsigned char>(c1.r + frac * (c2.r - c1.r)),
        static_cast<unsigned char>(c1.g + frac * (c2.g - c1.g)),
        static_cast<unsigned char>(c1.b + frac * (c2.b - c1.b)),
        255
    };
}
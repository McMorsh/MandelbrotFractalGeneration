#ifndef FARKTALGENERATION_COLORMAP_H
#define FARKTALGENERATION_COLORMAP_H


#pragma once
#include <raylib.h>
#include <vector>

class ColorMap {
private:
    std::vector<Color> colors;
public:
    ColorMap();
    Color getColor(int iter, int max_iter) const;
};


#endif //FARKTALGENERATION_COLORMAP_H
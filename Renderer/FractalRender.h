#ifndef FARKTALGENERATION_FRACTALRENDER_H
#define FARKTALGENERATION_FRACTALRENDER_H


#include "ColorMap.h"
#include <vector>

class FractalRender {
private:
    int width, height;
    Image image;
    Texture2D texture;
public:
    FractalRender(int width, int height);
    void draw(const std::vector<int>& data, ColorMap& colormap, int max_iter);
    void display();
};


#endif //FARKTALGENERATION_FRACTALRENDER_H
#include "Fractal.h"
#include "ColorMap.h"
#include "FractalRender.h"

int main() {
    int width = 1920;
    int height = 1080;
    int max_iter = 5000;

    Fractal fractal(max_iter);
    std::vector<int> data = fractal.generate(width, height);

    FractalRender renderer(width, height);
    ColorMap colormap;

    while (!WindowShouldClose()) {
        renderer.draw(data, colormap, max_iter);
        renderer.display();
    }

    return 0;
}
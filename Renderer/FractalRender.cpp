#include "FractalRender.h"

FractalRender::FractalRender(int width, int height)
    : width(width), height(height){

    InitWindow(width, height, "Mandelbrot Fractal (raylib)");
    SetTargetFPS(60);

    // Пустое изображение
    image = GenImageColor(width, height, BLACK);
    texture = LoadTextureFromImage(image);
}

void FractalRender::draw(const std::vector<int>& data, ColorMap& colormap, int max_iter) {
    // Проверка на размер
    if (data.size() != width * height) return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            // Проверка границ
            if (idx >= 0 && idx < static_cast<int>(data.size())) {
                Color color = colormap.getColor(data[idx], max_iter);
                ImageDrawPixel(&image, x, y, color);
            }
        }
    }

    UpdateTexture(texture, image.data);
}

void FractalRender::display() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexture(texture, 0, 0, WHITE);
    EndDrawing();
}


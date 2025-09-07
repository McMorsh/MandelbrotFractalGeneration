#include "FractalRender.h"

// Конструктор класса
// Создает окно raylib, настраивает FPS, инициализирует пустое изображение и текстуру
FractalRender::FractalRender(int width, int height)
    : width(width), height(height){

    InitWindow(width, height, "Mandelbrot Fractal (raylib)");
    SetTargetFPS(60);

    // Создаем пустое изображение нужного размера, закрашенное черным
    image = GenImageColor(width, height, BLACK);
    // Загружаем текстуру из изображения для последующего отображения
    texture = LoadTextureFromImage(image);
}

// Метод draw отрисовывает фрактал на изображении
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

    // Обновляем текстуру, чтобы изменения отображались на экране
    UpdateTexture(texture, image.data);
}

// Метод display выводит текстуру в окно
void FractalRender::display() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexture(texture, 0, 0, WHITE);
    EndDrawing();
}


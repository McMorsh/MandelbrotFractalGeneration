#ifndef MANDELBROT_FRACTAL_RENDER_H
#define MANDELBROT_FRACTAL_RENDER_H

#include "ColorMap.h"
#include <string>
#include <vector>

// Окно raylib, текстура с фракталом и экранный интерфейс.
class FractalRender {
private:
    int width, height;
    Image image{};
    Texture2D texture{};
public:
    FractalRender(int width, int height, const char* title);
    ~FractalRender();

    // Копирование запрещено: деструктор освобождает общие ресурсы raylib
    FractalRender(const FractalRender&) = delete;
    FractalRender& operator=(const FractalRender&) = delete;

    // Переносит карту итераций в изображение и обновляет текстуру.
    // maxIter задаёт порог принадлежности множеству.
    void draw(const std::vector<double>& data, const ColorMap& colormap, int maxIter);

    // Выводит текстуру и строку состояния; showHelp включает подсказки.
    void display(const std::string& info, bool showHelp) const;

    // Сохраняет изображение без интерфейса.
    bool saveImage(const std::string& path) const;
};

#endif //MANDELBROT_FRACTAL_RENDER_H

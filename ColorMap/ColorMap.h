#ifndef FARKTALGENERATION_COLORMAP_H
#define FARKTALGENERATION_COLORMAP_H


#pragma once
#include <raylib.h>
#include <vector>

// Класс отвечает за хранение палитры цветов
// и вычисление цвета для конкретного числа итераций при генерации фрактала
class ColorMap {
private:
    // Вектор цветов
    std::vector<Color> colors;
public:
    // Конструктор
    ColorMap();
    // Метод получения цвета для заданного номера итерации
    // Возвращает цвет типа Color (Raylib)
    Color getColor(int iter, int max_iter) const;
};


#endif //FARKTALGENERATION_COLORMAP_H
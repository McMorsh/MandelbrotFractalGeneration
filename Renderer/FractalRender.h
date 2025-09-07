#ifndef FARKTALGENERATION_FRACTALRENDER_H
#define FARKTALGENERATION_FRACTALRENDER_H


#include "ColorMap.h"
#include <vector>

// Класс FractalRender отвечает за визуализацию фрактала с использованием raylib
class FractalRender {
private:
    int width, height;      // размеры окна и изображения
    Image image;            // изображение raylib, на котором будут рисоваться пиксели
    Texture2D texture;      // текстура для отображения изображения в окне
public:
    // Конструктор класса
    FractalRender(int width, int height);
    // Метод отрисовки фрактала
    // data - массив чисел итераций для каждого пикселя
    // colormap - объект палитры цветов для преобразования итераций в цвета
    // max_iter - максимальное количество итераций для нормализации цвета
    void draw(const std::vector<int>& data, ColorMap& colormap, int max_iter);
    // Метод отображения изображения в окне
    void display();
};


#endif //FARKTALGENERATION_FRACTALRENDER_H
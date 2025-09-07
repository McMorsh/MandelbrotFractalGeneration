#include "ColorMap.h"

// Конструктор класса
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

// Метод получения цвета для точки фрактала в зависимости от числа итераций
Color ColorMap::getColor(int iter, int max_iter) const {
    // Если палитра пуста, возвращаем черный цвет
    if (colors.empty()) return BLACK;
    // Если точка дошла до максимального числа итераций (внутри множества)
    if (iter >= max_iter) return BLACK;

    // Нормализуем итерацию в диапазон 0..1
    float t = static_cast<float>(iter) / max_iter;
    // Определяем два индекса в палитре для интерполяции
    int idx1 = static_cast<int>(t * (colors.size() - 1));
    int idx2 = (idx1 + 1) % colors.size();
    // Вычисляем дробную часть для линейной интерполяции между цветами
    float frac = t * (colors.size() - 1) - idx1;

    // Получаем два цвета для интерполяции
    Color c1 = colors[idx1];
    Color c2 = colors[idx2];

    // Возвращаем интерполированный цвет
    return {
        static_cast<unsigned char>(c1.r + frac * (c2.r - c1.r)),
        static_cast<unsigned char>(c1.g + frac * (c2.g - c1.g)),
        static_cast<unsigned char>(c1.b + frac * (c2.b - c1.b)),
        255
    };
}
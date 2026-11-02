#pragma once

// Базовый диапазон числа итераций и шаг ручной подстройки.
inline constexpr int kMinIterations = 50;
inline constexpr int kMaxIterations = 5000;
inline constexpr int kIterationStep = 100;

/// Диапазон смещения числа итераций (подстройка колесом/клавишами).
inline constexpr int kMinIterationBonus = -2000;
inline constexpr int kMaxIterationBonus = 3000;

/// Подбирает число итераций по ширине окна в комплексных единицах.
int iterationsForSpan(double span);

/// Добавляет к автоматическому числу ручную подстройку и ограничивает результат.
int iterationsForView(double span, int bonus);

/// Проверяет, сдвинет ли добавка delta итоговое число итераций для вида span.
/// @param bonus текущая ручная добавка
/// @param nextBonus новое значение добавки
/// @return false на границах 50 и 5000, чтобы накопление не сдвигало рабочую точку, а обратное нажатие
/// срабатывало сразу.
bool nextIterationBonus(double span, int bonus, int delta, int& nextBonus);

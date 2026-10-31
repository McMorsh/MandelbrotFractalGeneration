#pragma once

// Пределы ручной подстройки итераций.
inline constexpr int kMinIterations = 50;
inline constexpr int kMaxIterations = 5000;
inline constexpr int kIterationStep = 100;
inline constexpr int kMinIterationBonus = -2000;
inline constexpr int kMaxIterationBonus = 3000;

// Автоматическое число итераций по ширине окна в комплексных единицах.
int iterationsForSpan(double span);

// Автоматическое число плюс ручная подстройка, с ограничением.
int iterationsForView(double span, int bonus);

// Проверяет, меняет ли добавка delta эффективное число итераций.
// На границах 50 и 5000 возвращает false, чтобы накопление не сдвигало
// рабочую точку и обратное нажатие срабатывало сразу.
bool nextIterationBonus(double span, int bonus, int delta, int& nextBonus);

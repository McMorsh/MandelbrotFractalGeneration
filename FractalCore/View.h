#pragma once

// Начальный вид целиком содержит множество Мандельброта
inline constexpr double kInitialCenterX = -0.6;
inline constexpr double kInitialCenterY = 0.0;
inline constexpr double kInitialSpan = 3.5;

// Ниже этого масштаба на типичной ширине окна double перестаёт
// различать соседние пиксели
inline constexpr double kMinSpan = 1e-11;
inline constexpr double kMaxSpan = 8.0;

// Точка комплексной плоскости
struct ComplexPoint {
	double real = 0.0;
	double imag = 0.0;
};

// Камера: центр окна комплексной плоскости, его ширина в комплексных
// единицах и размеры экрана. Размеры приходят снаружи, поэтому камеру
// можно тестировать без окна.
class View {
  public:
	// Начальный вид для экрана заданного размера
	View(int width, int height);
	// span приводится к диапазону [kMinSpan, kMaxSpan], а нечисловые
	// центр и span заменяются домашними значениями
	View(double centerX, double centerY, double span, int width, int height);

	double getCenterX() const;
	double getCenterY() const;
	double getSpan() const;
	int getWidth() const;
	int getHeight() const;

	// Меняет размер экрана, сохраняя центр и масштаб.
	// Некорректный размер игнорируется.
	void resize(int width, int height);

	// Точка комплексной плоскости под экранной точкой в пикселях
	ComplexPoint pointAt(double screenX, double screenY) const;

	// Масштабирует span в factor раз так, чтобы точка под курсором
	// осталась на месте. Возвращает false, если масштаб не изменился
	// или factor не положителен.
	bool zoomAt(double factor, double screenX, double screenY);

	// Сдвигает изображение за курсором, delta задаётся в пикселях
	void panBy(double deltaX, double deltaY);

  private:
	double centerX = kInitialCenterX;
	double centerY = kInitialCenterY;
	double span = kInitialSpan;
	int width = 1;
	int height = 1;
};

// Сравнивает центр и span; размеры экрана не учитываются
bool sameView(const View& lhs, const View& rhs);

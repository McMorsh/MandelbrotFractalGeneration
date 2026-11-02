#pragma once

#include "ColorMap.h"
#include "Localization.h"
#include <raylib.h>
#include <string>
#include <vector>

// Состояние панели с ползунками параметра c для Julia.
struct JuliaControls {
	bool visible = false;
	double real = 0.0;
	double imag = 0.0;
	int activeSlider = -1;
};

// Окно raylib, текстура с фракталом и экранный интерфейс.
class FractalRender {
  private:
	int width, height;
	Image image;
	Texture2D texture;
	Font font{};
	bool customFont = false;
	Language language = Language::Russian;
	FractalType fractalType = FractalType::Mandelbrot;

  public:
	FractalRender(int width, int height, const char* title);
	~FractalRender();

	// Texture2D и Image не освобождаются автоматически при копировании
	FractalRender(const FractalRender&) = delete;
	FractalRender& operator=(const FractalRender&) = delete;

	// Синхронизирует размер области вывода с клиентской областью окна.
	// Текстура пересоздаётся при следующем draw под размер новых данных.
	void resize(int width, int height);

	int getWidth() const {
		return width;
	}

	int getHeight() const {
		return height;
	}

	// Переносит карту итераций в изображение и обновляет текстуру.
	// dataWidth и dataHeight задают размер карты; текстура масштабируется
	// под окно. maxIter задаёт порог принадлежности множеству.
	void draw(const std::vector<double>& data, const ColorMap& colormap, int maxIter, int dataWidth, int dataHeight);

	// Выводит текстуру, строки состояния, уведомление и подсказки.
	// status показывается рядом с FPS, showHelp включает панель.
	void display(const std::string& info, const std::string& details, const std::string& notice,
				 const std::string& status, bool showHelp, const JuliaControls& juliaControls) const;

	// Индекс ползунка c под точкой: 0 для Re, 1 для Im, -1 если попадания нет.
	int juliaSliderAt(float x, float y) const;

	// true, если точка попадает в панель ползунков c.
	bool juliaPanelAt(float x, float y) const;

	// Значение ползунка c по координате x в диапазоне [-2, 2].
	double juliaSliderValue(int slider, float x) const;

	// Меняет язык заголовка окна и панели подсказок.
	void setLanguage(Language language);

	// Меняет тип фрактала в заголовке окна и HUD.
	void setFractalType(FractalType type);
};

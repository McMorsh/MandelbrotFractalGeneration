#pragma once

#include "ColorMap.h"
#include "FractalType.h"
#include "Localization.h"
#include <raylib.h>
#include <string>
#include <vector>

/// Состояние панели с ползунками параметра c для Julia.
struct JuliaControls {
	bool visible = false;
	/// -1: нет, 0: Re, 1: Im.
	int activeSlider = -1;
};

/// Окно raylib, текстура с фракталом и экранный интерфейс.
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
	/// Создаёт окно raylib заданного размера.
	FractalRender(int width, int height, const char* title);
	~FractalRender();

	/// Копирование запрещено: Texture2D и Image не освобождаются автоматически.
	FractalRender(const FractalRender&) = delete;
	FractalRender& operator=(const FractalRender&) = delete;

	/// Синхронизирует размер вывода с окном; текстура пересоздаётся при следующем draw.
	void resize(int width, int height);

	int getWidth() const {
		return width;
	}

	int getHeight() const {
		return height;
	}

	/// Переносит карту итераций в изображение и обновляет текстуру.
	/// @param data карта сглаженных итераций размера dataWidth * dataHeight
	/// @param maxIter порог, начиная с которого точка считается внутренней
	/// Текстура масштабируется под окно.
	void draw(const std::vector<double>& data, const ColorMap& colormap, int maxIter, int dataWidth, int dataHeight);

	/// Выводит текстуру, строки состояния, уведомление и подсказки.
	/// @param info зум, итерации, палитра, время кадра и пресет PNG
	/// @param details центр вида и точка под курсором
	/// @param notice временное уведомление
	/// @param status состояние фонового расчёта
	/// @param showHelp включает панель подсказок
	/// @param juliaControls ползунки параметра c
	void display(const std::string& info, const std::string& details, const std::string& notice,
				 const std::string& status, bool showHelp, const JuliaControls& juliaControls,
				 const FractalParams& fractal) const;

	/// Индекс ползунка c под точкой.
	/// @return 0 для Re, 1 для Im, -1 если попадания нет.
	int juliaSliderAt(float x, float y) const;

	/// Возвращает true, если точка попала в панель ползунков c.
	bool juliaPanelAt(float x, float y) const;

	/// Значение ползунка c по координате x.
	/// @param slider 0 для Re, 1 для Im
	/// @return значение в диапазоне [-2, 2].
	double juliaSliderValue(int slider, float x) const;

	/// Меняет язык заголовка окна и панели подсказок.
	void setLanguage(Language language);

	/// Меняет тип фрактала в заголовке окна и HUD.
	void setFractalType(FractalType type);
};

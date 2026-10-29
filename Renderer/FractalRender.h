#pragma once

#include "ColorMap.h"
#include <raylib.h>
#include <string>
#include <vector>

// Окно raylib, текстура с фракталом и экранный интерфейс.
class FractalRender {
  private:
	int width, height;
	Image image;
	Texture2D texture;

  public:
	FractalRender(int width, int height, const char* title);
	~FractalRender();

	// Texture2D и Image не освобождаются автоматически при копировании
	FractalRender(const FractalRender&) = delete;
	FractalRender& operator=(const FractalRender&) = delete;

	// Переносит карту итераций в изображение и обновляет текстуру.
	// dataWidth и dataHeight задают размер карты; текстура масштабируется
	// под окно. maxIter задаёт порог принадлежности множеству.
	void draw(const std::vector<double>& data, const ColorMap& colormap, int maxIter, int dataWidth, int dataHeight);

	// Выводит текстуру, две строки состояния и подсказки при showHelp.
	void display(const std::string& info, const std::string& details, bool showHelp) const;

	// Сохраняет текущее изображение без интерфейса.
	bool saveImage(const std::string& path) const;
};

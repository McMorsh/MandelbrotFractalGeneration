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
	// busy показывает индикатор фонового расчёта, showHelp включает панель.
	void display(const std::string& info, const std::string& details, const std::string& notice, bool busy,
				 bool showHelp) const;

	// Сохраняет текущее изображение без интерфейса.
	bool saveImage(const std::string& path) const;
};

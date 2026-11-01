#pragma once

#include "ColorMap.h"
#include "Localization.h"
#include <raylib.h>
#include <string>
#include <vector>

// Окно raylib, текстура с фракталом и экранный интерфейс.
class FractalRender {
  private:
	int width, height;
	Image image;
	Texture2D texture;
	Font font{};
	bool customFont = false;
	Language language = Language::Russian;

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
				 const std::string& status, bool showHelp) const;

	// Меняет язык заголовка окна и панели подсказок.
	void setLanguage(Language language);

	// Сохраняет текущее изображение без интерфейса.
	bool saveImage(const std::string& path) const;
};

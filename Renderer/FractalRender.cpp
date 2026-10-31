#include "FractalRender.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>

namespace {
	constexpr int kMinWindowWidth = 640;
	constexpr int kMinWindowHeight = 480;
	constexpr int kTopBarHeight = 96;
	constexpr int kPanelWidth = 330;
	constexpr int kPanelPadding = 16;
	constexpr int kLineHeight = 20;

	constexpr const char* kHelpLines[] = {
		"Mouse wheel     zoom in / out", "Left drag       pan",		   "[ / ]           iterations - / +",
		"P               next palette",	 "R               reset view", "F12             save PNG",
		"H               toggle help",	 "Esc             quit",
	};

	constexpr int kPanelHeight = kPanelPadding * 2 + 28 + static_cast<int>(std::size(kHelpLines)) * kLineHeight;

	// Обрезает текст посередине, чтобы сохранить начало и имя файла в конце
	std::string fitText(const std::string& text, int maxWidth, int fontSize) {
		if (MeasureText(text.c_str(), fontSize) <= maxWidth)
			return text;

		constexpr const char* kDots = "...";
		std::size_t keepLeft = text.size() / 2;
		std::size_t keepRight = text.size() - keepLeft;

		while (keepLeft > 0 && keepRight > 0) {
			const std::string candidate = text.substr(0, keepLeft) + kDots + text.substr(text.size() - keepRight);
			if (MeasureText(candidate.c_str(), fontSize) <= maxWidth)
				return candidate;

			if (keepLeft > keepRight)
				--keepLeft;
			else
				--keepRight;
		}

		return kDots;
	}
} // namespace

FractalRender::FractalRender(int width, int height, const char* title) {
	InitWindow(width, height, title);
	SetTargetFPS(60);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetWindowMinSize(kMinWindowWidth, kMinWindowHeight);

	// Берём фактический размер: ОС может скорректировать запрошенный
	this->width = GetScreenWidth();
	this->height = GetScreenHeight();

	image = GenImageColor(this->width, this->height, BLACK);
	texture = LoadTextureFromImage(image);
	// Без сглаживания пиксели фрактала остаются чёткими
	SetTextureFilter(texture, TEXTURE_FILTER_POINT);
}

FractalRender::~FractalRender() {
	UnloadTexture(texture);
	UnloadImage(image);
	CloseWindow();
}

void FractalRender::resize(int width, int height) {
	if (width <= 0 || height <= 0)
		return;

	this->width = width;
	this->height = height;
}

void FractalRender::draw(const std::vector<double>& data, const ColorMap& colormap, int maxIter, int dataWidth,
						 int dataHeight) {
	if (dataWidth <= 0 || dataHeight <= 0)
		return;
	if (data.size() != static_cast<std::size_t>(dataWidth) * dataHeight)
		return;

	// Предпросмотр и полный кадр имеют разный размер, поэтому текстура
	// пересоздаётся только при смене разрешения.
	if (image.width != dataWidth || image.height != dataHeight) {
		UnloadTexture(texture);
		UnloadImage(image);

		image = GenImageColor(dataWidth, dataHeight, BLACK);
		texture = LoadTextureFromImage(image);
		SetTextureFilter(texture, TEXTURE_FILTER_POINT);
	}

	// GenImageColor создаёт RGBA-изображение, поэтому можно писать напрямую
	auto* pixels = static_cast<Color*>(image.data);

#pragma omp parallel for schedule(static)
	for (int y = 0; y < dataHeight; y++) {
		for (int x = 0; x < dataWidth; x++) {
			const std::size_t idx = static_cast<std::size_t>(y) * dataWidth + x;
			pixels[idx] = colormap.getColor(data[idx], maxIter);
		}
	}

	UpdateTexture(texture, image.data);
}

void FractalRender::display(const std::string& info, const std::string& details, const std::string& notice, bool busy,
							bool showHelp) const {
	BeginDrawing();
	ClearBackground(BLACK);

	// Кадр может быть меньше окна, если сейчас показан предпросмотр
	const Rectangle source = {0.0f, 0.0f, static_cast<float>(image.width), static_cast<float>(image.height)};
	const Rectangle dest = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)};
	DrawTexturePro(texture, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);

	// Встроенный шрифт raylib не содержит кириллицы, поэтому интерфейс на английском
	DrawRectangle(0, 0, width, kTopBarHeight, Fade(BLACK, 0.55f));
	DrawText("MANDELBROT FRACTAL", 24, 14, 24, RAYWHITE);

	// Строки, которые не помещаются в узкое окно, не рисуем,
	// иначе они вылезают за край или накладываются на счётчик FPS
	const int textLimit = width - 48;
	if (MeasureText(info.c_str(), 16) <= textLimit) {
		DrawText(info.c_str(), 24, 46, 16, Fade(RAYWHITE, 0.72f));
	}

	bool noticeShown = false;
	if (!notice.empty()) {
		// Уведомление важнее координат: обрезаем его, но не скрываем
		const std::string noticeText = fitText(notice, textLimit, 16);
		DrawText(noticeText.c_str(), 24, 68, 16, Fade(RAYWHITE, 0.85f));
		noticeShown = true;
	}
	if (!noticeShown && MeasureText(details.c_str(), 16) <= textLimit) {
		DrawText(details.c_str(), 24, 68, 16, Fade(RAYWHITE, 0.55f));
	}

	const char* fps = busy ? TextFormat("%d FPS   |   rendering...", GetFPS()) : TextFormat("%d FPS", GetFPS());
	const int fpsWidth = MeasureText(fps, 16);
	DrawText(fps, width - fpsWidth - 24, 22, 16, Fade(RAYWHITE, 0.6f));

	if (showHelp) {
		// При маленькой высоте панель поднимается вплотную к верхней строке
		const int panelX = 24;
		const int panelY = std::max(height - kPanelHeight - 24, kTopBarHeight + 12);

		DrawRectangle(panelX, panelY, kPanelWidth, kPanelHeight, Fade(BLACK, 0.55f));
		DrawText("CONTROLS", panelX + kPanelPadding, panelY + kPanelPadding, 16, Fade(RAYWHITE, 0.55f));

		int lineY = panelY + kPanelPadding + 28;
		for (const char* line : kHelpLines) {
			DrawText(line, panelX + kPanelPadding, lineY, 16, RAYWHITE);
			lineY += kLineHeight;
		}
	}

	EndDrawing();
}

bool FractalRender::saveImage(const std::string& path) const {
	return ExportImage(image, path.c_str());
}

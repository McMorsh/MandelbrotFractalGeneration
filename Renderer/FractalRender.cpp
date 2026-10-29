#include "FractalRender.h"

#include <cstddef>
#include <iterator>

namespace {
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
} // namespace

FractalRender::FractalRender(int width, int height, const char* title) : width(width), height(height) {

	InitWindow(width, height, title);
	SetTargetFPS(60);

	image = GenImageColor(width, height, BLACK);
	texture = LoadTextureFromImage(image);
	// Без сглаживания пиксели фрактала остаются чёткими
	SetTextureFilter(texture, TEXTURE_FILTER_POINT);
}

FractalRender::~FractalRender() {
	UnloadTexture(texture);
	UnloadImage(image);
	CloseWindow();
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

void FractalRender::display(const std::string& info, const std::string& details, bool showHelp) const {
	BeginDrawing();
	ClearBackground(BLACK);

	// Кадр может быть меньше окна, если сейчас показан предпросмотр
	const Rectangle source = {0.0f, 0.0f, static_cast<float>(image.width), static_cast<float>(image.height)};
	const Rectangle dest = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)};
	DrawTexturePro(texture, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);

	// Встроенный шрифт raylib не содержит кириллицы, поэтому интерфейс на английском
	DrawRectangle(0, 0, width, kTopBarHeight, Fade(BLACK, 0.55f));
	DrawText("MANDELBROT FRACTAL", 24, 14, 24, RAYWHITE);
	DrawText(info.c_str(), 24, 46, 16, Fade(RAYWHITE, 0.72f));
	DrawText(details.c_str(), 24, 68, 16, Fade(RAYWHITE, 0.55f));

	const char* fps = TextFormat("%d FPS", GetFPS());
	const int fpsWidth = MeasureText(fps, 16);
	DrawText(fps, width - fpsWidth - 24, 22, 16, Fade(RAYWHITE, 0.6f));

	if (showHelp) {
		const int panelX = 24;
		const int panelY = height - kPanelHeight - 24;

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

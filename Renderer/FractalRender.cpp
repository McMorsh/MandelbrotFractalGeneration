#include "FractalRender.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace {
	constexpr int kMinWindowWidth = 640;
	constexpr int kMinWindowHeight = 480;
	constexpr int kTopBarHeight = 96;
	constexpr int kPanelWidth = 380;
	constexpr int kPanelPadding = 16;
	constexpr int kLineHeight = 20;
	constexpr int kHelpKeyColumnWidth = 170;
	constexpr int kFontAtlasSize = 32;
	constexpr int kTitleFontSize = 24;
	constexpr int kHudFontSize = 16;
	constexpr float kTextSpacing = 0.0f;

	constexpr int kPanelHeight = kPanelPadding * 2 + 28 + kHelpLineCount * kLineHeight;

	std::string assetPath(const char* name) {
		std::string directory = GetApplicationDirectory();
		if (!directory.empty() && directory.back() != '/' && directory.back() != '\\')
			directory += '/';
		return directory + "assets/" + name;
	}

	Font loadInterfaceFont(bool& customFont) {
		customFont = false;

		// ASCII и кириллица; диапазон покрывает русские буквы и «ё»
		std::vector<int> codepoints;
		codepoints.reserve(95 + 256);
		for (int code = 32; code < 127; ++code)
			codepoints.push_back(code);
		for (int code = 0x0400; code <= 0x04FF; ++code)
			codepoints.push_back(code);

		const std::string path = assetPath("IBMPlexMono-Regular.ttf");
		Font loaded = LoadFontEx(path.c_str(), kFontAtlasSize, codepoints.data(), static_cast<int>(codepoints.size()));
		if (loaded.texture.id == 0) {
			TraceLog(LOG_WARNING, "Font not found: %s", path.c_str());
			return GetFontDefault();
		}

		customFont = true;
		SetTextureFilter(loaded.texture, TEXTURE_FILTER_BILINEAR);
		return loaded;
	}

	void drawUiText(const Font& font, const char* text, int x, int y, int fontSize, Color color) {
		DrawTextEx(font, text, Vector2{static_cast<float>(x), static_cast<float>(y)}, static_cast<float>(fontSize),
				   kTextSpacing, color);
	}

	int measureUiText(const Font& font, const char* text, int fontSize) {
		return static_cast<int>(MeasureTextEx(font, text, static_cast<float>(fontSize), kTextSpacing).x);
	}

	std::size_t utf8Length(const std::string& text) {
		std::size_t length = 0;
		for (const unsigned char byte : text) {
			if ((byte & 0xC0) != 0x80)
				++length;
		}
		return length;
	}

	std::size_t utf8Offset(const std::string& text, std::size_t codePointIndex) {
		std::size_t index = 0;
		for (std::size_t i = 0; i < text.size(); ++i) {
			if ((static_cast<unsigned char>(text[i]) & 0xC0) != 0x80) {
				if (index == codePointIndex)
					return i;
				++index;
			}
		}
		return text.size();
	}

	// Обрезает текст посередине, не разрывая UTF-8 символы
	std::string fitText(const Font& font, const std::string& text, int maxWidth, int fontSize) {
		if (measureUiText(font, text.c_str(), fontSize) <= maxWidth)
			return text;

		constexpr const char* kDots = "...";
		const std::size_t length = utf8Length(text);
		std::size_t keepLeft = length / 2;
		std::size_t keepRight = length - keepLeft;

		while (keepLeft > 0 && keepRight > 0) {
			const std::string candidate =
				text.substr(0, utf8Offset(text, keepLeft)) + kDots + text.substr(utf8Offset(text, length - keepRight));
			if (measureUiText(font, candidate.c_str(), fontSize) <= maxWidth)
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

	font = loadInterfaceFont(customFont);

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
	if (customFont)
		UnloadFont(font);
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

void FractalRender::display(const std::string& info, const std::string& details, const std::string& notice,
							const std::string& status, bool showHelp) const {
	BeginDrawing();
	ClearBackground(BLACK);

	// Кадр может быть меньше окна, если сейчас показан предпросмотр
	const Rectangle source = {0.0f, 0.0f, static_cast<float>(image.width), static_cast<float>(image.height)};
	const Rectangle dest = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)};
	DrawTexturePro(texture, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);

	DrawRectangle(0, 0, width, kTopBarHeight, Fade(BLACK, 0.55f));
	const UiText& text = uiText(language);
	drawUiText(font, text.appTitle, 24, 14, kTitleFontSize, RAYWHITE);

	// Строки, которые не помещаются в узкое окно, не рисуем,
	// иначе они вылезают за край или накладываются на счётчик FPS
	const int textLimit = width - 48;
	if (measureUiText(font, info.c_str(), kHudFontSize) <= textLimit) {
		drawUiText(font, info.c_str(), 24, 46, kHudFontSize, Fade(RAYWHITE, 0.72f));
	}

	bool noticeShown = false;
	if (!notice.empty()) {
		// Уведомление важнее координат: обрезаем его, но не скрываем
		const std::string noticeText = fitText(font, notice, textLimit, kHudFontSize);
		drawUiText(font, noticeText.c_str(), 24, 68, kHudFontSize, Fade(RAYWHITE, 0.85f));
		noticeShown = true;
	}
	if (!noticeShown && measureUiText(font, details.c_str(), kHudFontSize) <= textLimit) {
		drawUiText(font, details.c_str(), 24, 68, kHudFontSize, Fade(RAYWHITE, 0.55f));
	}

	const char* fps =
		status.empty() ? TextFormat("%d FPS", GetFPS()) : TextFormat("%d FPS   |   %s", GetFPS(), status.c_str());
	const int fpsWidth = measureUiText(font, fps, kHudFontSize);
	drawUiText(font, fps, width - fpsWidth - 24, 22, kHudFontSize, Fade(RAYWHITE, 0.6f));

	if (showHelp) {
		// При маленькой высоте панель поднимается вплотную к верхней строке
		const int panelX = 24;
		const int panelY = std::max(height - kPanelHeight - 24, kTopBarHeight + 12);

		DrawRectangle(panelX, panelY, kPanelWidth, kPanelHeight, Fade(BLACK, 0.55f));
		drawUiText(font, text.controlsHeader, panelX + kPanelPadding, panelY + kPanelPadding, kHudFontSize,
				   Fade(RAYWHITE, 0.55f));

		const HelpLineText* lines = helpLines(language);
		int lineY = panelY + kPanelPadding + 28;
		const int keyX = panelX + kPanelPadding;
		const int actionX = keyX + kHelpKeyColumnWidth;
		for (int i = 0; i < kHelpLineCount; ++i) {
			drawUiText(font, lines[i].keys, keyX, lineY, kHudFontSize, RAYWHITE);
			drawUiText(font, lines[i].action, actionX, lineY, kHudFontSize, Fade(RAYWHITE, 0.75f));
			lineY += kLineHeight;
		}
	}

	EndDrawing();
}

bool FractalRender::saveImage(const std::string& path) const {
	return ExportImage(image, path.c_str());
}

void FractalRender::setLanguage(Language language) {
	this->language = language;
	SetWindowTitle(uiText(language).windowTitle);
}

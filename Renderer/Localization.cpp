#include "Localization.h"

#include <iterator>

namespace {
	constexpr UiText kRussianText = {
		"УПРАВЛЕНИЕ",
		"масштаб %.3gx   |   итераций %d   |   %s   | рендер %.1f мс   |   PNG %s",
		"центр (%.15g, %+.15gi)   |   курсор (%.15g, %+.15gi)",
		"курсор (%.15g, %+.15gi)",
		"   |   время с рендера %.0f мс",
		"Скопировано в буфер обмена",
		"В буфере обмена нет параметров вида",
		"Неизвестная палитра в буфере обмена",
		"Не удалось восстановить итерации",
		"Вид вставлен",
		"Сохранение уже идёт",
		"Рендер %s...",
		"Сохранено ",
		"Не удалось сохранить ",
		"рендер",
	};

	constexpr UiText kEnglishText = {
		"CONTROLS",
		"zoom %.3gx   |   iterations %d   |   %s   |   render %.1f ms   |   PNG %s",
		"center (%.15g, %+.15gi)   |   cursor (%.15g, %+.15gi)",
		"cursor (%.15g, %+.15gi)",
		"   |   image age %.0f ms",
		"Copied to clipboard",
		"Clipboard has no view data",
		"Unknown palette in clipboard",
		"Cannot restore iterations",
		"Pasted view",
		"Save already in progress",
		"Rendering %s...",
		"Saved ",
		"Could not save ",
		"rendering",
	};

	constexpr HelpLineText kRussianHelp[] = {
		{"Колесо, + / -", "управление масштабом"},
		{"Стрелки, ЛКМ", "панорама"},
		{"[ / ]", "итерации"},
		{"P", "палитра"},
		{"C / V", "копировать / вставить"},
		{"F2", "сохранить PNG"},
		{"Shift + влево / вправо", "размер PNG"},
		{"T", "тип фрактала"},
		{"R", "сброс вида"},
		{"L", "локализация"},
		{"H", "справка"},
		{"Esc", "выход"},
	};

	constexpr HelpLineText kEnglishHelp[] = {
		{"Wheel, + / -", "zoom"},
		{"Arrows, LMB", "pan"},
		{"[ / ]", "iterations"},
		{"P", "palette"},
		{"C / V", "copy / paste"},
		{"F2", "save PNG"},
		{"Shift + Left / Right", "PNG size"},
		{"T", "fractal type"},
		{"R", "reset view"},
		{"L", "language"},
		{"H", "help"},
		{"Esc", "quit"},
	};

	static_assert(std::size(kRussianHelp) == kHelpLineCount);
	static_assert(std::size(kEnglishHelp) == kHelpLineCount);
} // namespace

Language nextLanguage(Language language) {
	return language == Language::Russian ? Language::English : Language::Russian;
}

const UiText& uiText(Language language) {
	return language == Language::Russian ? kRussianText : kEnglishText;
}

const HelpLineText* helpLines(Language language) {
	return language == Language::Russian ? kRussianHelp : kEnglishHelp;
}

const char* fractalTitle(FractalType type, Language language) {
	if (language == Language::Russian)
		return type == FractalType::Julia ? "Множество Жюлиа" : "Множество Мандельброта";

	return type == FractalType::Julia ? "Julia Set" : "Mandelbrot Set";
}

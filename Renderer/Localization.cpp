#include "Localization.h"

#include <iterator>

namespace {
	constexpr UiText kRussianText = {
		"Генерация множества Мандельброта",
		"ФРАКТАЛ МАНДЕЛЬБРОТА",
		"УПРАВЛЕНИЕ",
		"масштаб %.3gx   |   итераций %d   |   %s   | рендер %.1f мс",
		"центр (%.15g, %+.15gi)   |   курсор (%.15g, %+.15gi)",
		"курсор (%.15g, %+.15gi)",
		"   |   время с рендера %.0f мс",
		"Скопировано в буфер обмена",
		"В буфере обмена нет параметров вида",
		"Неизвестная палитра в буфере обмена",
		"Не удалось восстановить итерации",
		"Вид вставлен",
		"Сохранение отменено: вид изменён",
		"Рендер ещё не завершён",
		"Ожидание полного рендера",
		"Сохранение уже идёт",
		"Изображение 2x слишком большое",
		"Рендер 2x...",
		"Сохранено ",
		"Не удалось сохранить ",
		"рендер",
		"PNG 2x",
	};

	constexpr UiText kEnglishText = {
		"Mandelbrot Fractal Generation",
		"MANDELBROT FRACTAL",
		"CONTROLS",
		"zoom %.3gx   |   iterations %d   |   %s   |   render %.1f ms",
		"center (%.15g, %+.15gi)   |   cursor (%.15g, %+.15gi)",
		"cursor (%.15g, %+.15gi)",
		"   |   image age %.0f ms",
		"Copied to clipboard",
		"Clipboard has no view data",
		"Unknown palette in clipboard",
		"Cannot restore iterations",
		"Pasted view",
		"Save canceled: view changed",
		"Nothing rendered yet",
		"Waiting for full render",
		"Save already in progress",
		"2x image is too large",
		"Rendering 2x...",
		"Saved ",
		"Could not save ",
		"rendering",
		"PNG 2x",
	};

	constexpr HelpLineText kRussianHelp[] = {
		{"Колесо, + / -", "управление масштабом"},
		{"Стрелки, ЛКМ", "панорама"},
		{"[ / ]", "итерации"},
		{"P", "палитра"},
		{"C / V", "копировать / вставить"},
		{"F2 / Shift + F2", "PNG / PNG 2x"},
		{"R", "сброс вида"},
		{"L", "локализация"},
		{"H", "справка"},
		{"Esc", "выход"},
	};

	constexpr HelpLineText kEnglishHelp[] = {
		{"Wheel, + / -", "zoom"}, {"Arrows, LMB", "pan"},	 {"[ / ]", "iterations"},
		{"P", "palette"},		  {"C / V", "copy / paste"}, {"F2 / Shift + F2", "PNG / PNG 2x"},
		{"R", "reset view"},	  {"L", "language"},		 {"H", "help"},
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

#pragma once

// Язык интерфейса.
enum class Language {
	Russian,
	English,
};

// Фиксированные строки интерфейса. Форматные строки рассчитаны на snprintf.
struct UiText {
	const char* windowTitle;
	const char* appTitle;
	const char* controlsHeader;
	const char* infoFormat;
	const char* detailsFormat;
	const char* detailsNarrowFormat;
	const char* imageAgeFormat;
	const char* copied;
	const char* clipboardNoView;
	const char* unknownPalette;
	const char* cannotRestoreIterations;
	const char* pastedView;
	const char* saveCanceled;
	const char* nothingRendered;
	const char* waitingFullRender;
	const char* saveInProgress;
	const char* imageTooLarge;
	const char* rendering2x;
	const char* savedPrefix;
	const char* saveFailedPrefix;
	const char* statusRendering;
	const char* statusPng2x;
};

// Строка панели подсказок: клавиши слева, действие справа.
struct HelpLineText {
	const char* keys;
	const char* action;
};

inline constexpr int kHelpLineCount = 10;

// Возвращает противоположный язык.
Language nextLanguage(Language language);

// Возвращает строки выбранного языка.
const UiText& uiText(Language language);

// Возвращает массив строк панели подсказок выбранного языка.
const HelpLineText* helpLines(Language language);

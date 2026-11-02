#pragma once

#include "FractalType.h"

/// Язык интерфейса.
enum class Language {
	Russian,
	English,
};

/// Фиксированные строки интерфейса. Форматные строки рассчитаны на snprintf.
struct UiText {
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
	const char* saveInProgress;
	const char* renderingExportFormat;
	const char* savedPrefix;
	const char* saveFailedPrefix;
	const char* statusRendering;
};

/// Строка панели подсказок: клавиши слева, действие справа.
struct HelpLineText {
	const char* keys;
	const char* action;
};

/// Число строк панели подсказок.
inline constexpr int kHelpLineCount = 12;

/// Название фрактала для заголовка окна и HUD.
const char* fractalTitle(FractalType type, Language language);

/// Возвращает противоположный язык.
Language nextLanguage(Language language);

/// Возвращает строки выбранного языка.
const UiText& uiText(Language language);

/// Возвращает массив строк панели подсказок выбранного языка.
const HelpLineText* helpLines(Language language);

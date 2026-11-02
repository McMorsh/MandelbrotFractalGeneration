#pragma once

#include "FractalType.h"
#include "Localization.h"

#include <filesystem>
#include <string>

// Настройки интерфейса, которые сохраняются между запусками.
struct Settings {
	Language language = Language::Russian;
	// Имя палитры, по умолчанию совпадает с первой палитрой ColorMap
	std::string palette = "Ember";
	// Метка пресета PNG, по умолчанию совпадает с FHD
	std::string exportPreset = "FHD";
	FractalParams fractal;
};

// Возвращает путь к файлу настроек пользователя.
// Windows: %APPDATA%\FractalGeneration\settings.txt.
// Linux: $XDG_CONFIG_HOME/FractalGeneration/settings.txt,
// иначе ~/.config/FractalGeneration/settings.txt.
std::filesystem::path settingsPath();

// Путь к настройкам до переименования проекта.
// Нужен, чтобы перенести файл при первом запуске.
std::filesystem::path legacySettingsPath();

// Разбирает содержимое файла настроек.
// Неизвестные ключи и значения пропускаются.
Settings parseSettings(const std::string& text);

// Сериализует настройки в текст вида "language=ru\n".
std::string formatSettings(const Settings& settings);

// Читает настройки. Если файла нет, возвращает значения по умолчанию.
Settings loadSettings(const std::filesystem::path& path);

// Читает настройки. Если файла нет, пробует legacyPath и переносит его.
Settings loadSettings(const std::filesystem::path& path, const std::filesystem::path& legacyPath);

// Записывает настройки, создавая каталог при необходимости.
// Возвращает false, если файл не удалось создать или записать.
bool saveSettings(const Settings& settings, const std::filesystem::path& path);

#pragma once

#include "Localization.h"

#include <filesystem>
#include <string>

// Настройки интерфейса, которые сохраняются между запусками.
struct Settings {
	Language language = Language::Russian;
	// Метка пресета PNG, по умолчанию совпадает с FHD
	std::string exportPreset = "FHD";
};

// Возвращает путь к файлу настроек пользователя.
// Windows: %APPDATA%\MandelbrotFractalGeneration\settings.txt.
// Linux: $XDG_CONFIG_HOME/MandelbrotFractalGeneration/settings.txt,
// иначе ~/.config/MandelbrotFractalGeneration/settings.txt.
std::filesystem::path settingsPath();

// Разбирает содержимое файла настроек.
// Неизвестные ключи и значения пропускаются.
Settings parseSettings(const std::string& text);

// Сериализует настройки в текст вида "language=ru\n".
std::string formatSettings(const Settings& settings);

// Читает настройки. Если файла нет, возвращает значения по умолчанию.
Settings loadSettings(const std::filesystem::path& path);

// Записывает настройки, создавая каталог при необходимости.
// Возвращает false, если файл не удалось создать или записать.
bool saveSettings(const Settings& settings, const std::filesystem::path& path);

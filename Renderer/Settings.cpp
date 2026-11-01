#include "Settings.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

namespace {
	constexpr const char* kLanguageKey = "language=";
	constexpr const char* kRussianValue = "ru";
	constexpr const char* kEnglishValue = "en";
	constexpr const char* kConfigDirectory = "MandelbrotFractalGeneration";
	constexpr const char* kSettingsFileName = "settings.txt";

	std::filesystem::path currentDirectory() {
		std::error_code error;
		const std::filesystem::path path = std::filesystem::current_path(error);
		return error ? std::filesystem::path(".") : path;
	}

	std::filesystem::path configRoot() {
#if defined(_WIN32)
		if (const char* appData = std::getenv("APPDATA"); appData != nullptr && *appData != '\0')
			return appData;
#else
		if (const char* xdgConfig = std::getenv("XDG_CONFIG_HOME"); xdgConfig != nullptr && *xdgConfig != '\0')
			return xdgConfig;

		if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0')
			return std::filesystem::path(home) / ".config";
#endif
		// Переменные окружения не заданы: пишем рядом с текущим каталогом
		return currentDirectory();
	}
} // namespace

std::filesystem::path settingsPath() {
	return configRoot() / kConfigDirectory / kSettingsFileName;
}

Settings parseSettings(const std::string& text) {
	Settings settings;
	std::istringstream stream(text);
	std::string line;

	while (std::getline(stream, line)) {
		// ofstream в текстовом режиме на Windows пишет CRLF
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		if (!line.starts_with(kLanguageKey))
			continue;

		const std::string value = line.substr(std::string(kLanguageKey).size());
		if (value == kRussianValue)
			settings.language = Language::Russian;
		else if (value == kEnglishValue)
			settings.language = Language::English;
	}

	return settings;
}

std::string formatSettings(const Settings& settings) {
	const char* value = settings.language == Language::English ? kEnglishValue : kRussianValue;
	return std::string(kLanguageKey) + value + "\n";
}

Settings loadSettings(const std::filesystem::path& path) {
	std::ifstream file(path);
	if (!file.is_open())
		return Settings{};

	std::ostringstream text;
	text << file.rdbuf();
	return parseSettings(text.str());
}

bool saveSettings(const Settings& settings, const std::filesystem::path& path) {
	const std::filesystem::path directory = path.parent_path();
	if (!directory.empty()) {
		std::error_code error;
		std::filesystem::create_directories(directory, error);
		if (error)
			return false;
	}

	std::ofstream file(path, std::ios::trunc);
	if (!file.is_open())
		return false;

	file << formatSettings(settings);
	file.flush();
	return file.good();
}

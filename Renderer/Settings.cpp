#include "Settings.h"

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

namespace {
	constexpr const char* kLanguageKey = "language=";
	constexpr const char* kPaletteKey = "palette=";
	constexpr const char* kFractalKey = "fractal=";
	constexpr const char* kJuliaRealKey = "julia-re=";
	constexpr const char* kJuliaImagKey = "julia-im=";
	constexpr const char* kExportPresetKey = "png=";
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

	bool parseDoubleValue(const std::string& text, double& value) {
		errno = 0;
		char* end = nullptr;
		const double parsed = std::strtod(text.c_str(), &end);
		if (end == text.c_str() || *end != '\0' || errno == ERANGE || !std::isfinite(parsed))
			return false;

		value = parsed;
		return true;
	}

	std::string formatDouble(double value) {
		char buffer[32];
		std::snprintf(buffer, sizeof(buffer), "%.15g", value);
		return buffer;
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

		if (line.starts_with(kLanguageKey)) {
			const std::string value = line.substr(std::string(kLanguageKey).size());
			if (value == kRussianValue)
				settings.language = Language::Russian;
			else if (value == kEnglishValue)
				settings.language = Language::English;
		} else if (line.starts_with(kPaletteKey)) {
			const std::string value = line.substr(std::string(kPaletteKey).size());
			if (!value.empty())
				settings.palette = value;
		} else if (line.starts_with(kFractalKey)) {
			parseFractalType(line.substr(std::string(kFractalKey).size()), settings.fractal.type);
		} else if (line.starts_with(kJuliaRealKey)) {
			double value = 0.0;
			if (parseDoubleValue(line.substr(std::string(kJuliaRealKey).size()), value))
				settings.fractal.juliaReal = value;
		} else if (line.starts_with(kJuliaImagKey)) {
			double value = 0.0;
			if (parseDoubleValue(line.substr(std::string(kJuliaImagKey).size()), value))
				settings.fractal.juliaImag = value;
		} else if (line.starts_with(kExportPresetKey)) {
			const std::string value = line.substr(std::string(kExportPresetKey).size());
			if (!value.empty())
				settings.exportPreset = value;
		}
	}

	return settings;
}

std::string formatSettings(const Settings& settings) {
	const char* language = settings.language == Language::English ? kEnglishValue : kRussianValue;
	std::string text = std::string(kLanguageKey) + language + "\n";
	text += std::string(kPaletteKey) + settings.palette + "\n";
	text += std::string(kFractalKey) + fractalTypeName(settings.fractal.type) + "\n";
	text += std::string(kJuliaRealKey) + formatDouble(settings.fractal.juliaReal) + "\n";
	text += std::string(kJuliaImagKey) + formatDouble(settings.fractal.juliaImag) + "\n";
	text += std::string(kExportPresetKey) + settings.exportPreset + "\n";
	return text;
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

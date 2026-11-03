#include "settings.hpp"

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

#include "fractal_type.hpp"
#include "localization.hpp"

namespace {

constexpr const char* kLanguageKey = "language=";
constexpr const char* kPaletteKey = "palette=";
constexpr const char* kFractalKey = "fractal=";
constexpr const char* kJuliaRealKey = "julia-re=";
constexpr const char* kJuliaImagKey = "julia-im=";
constexpr const char* kExportPresetKey = "png=";
constexpr const char* kRussianValue = "ru";
constexpr const char* kEnglishValue = "en";
constexpr const char* kConfigDirectory = "FractalGeneration";
constexpr const char* kLegacyConfigDirectory = "MandelbrotFractalGeneration";
constexpr const char* kSettingsFileName = "settings.txt";

std::filesystem::path GetCurrentDirectory() {
  std::error_code error;
  const std::filesystem::path path = std::filesystem::current_path(error);
  return error ? std::filesystem::path(".") : path;
}

std::filesystem::path GetConfigRoot() {
#if defined(_WIN32)
  if (const char* app_data = std::getenv("APPDATA");
      app_data != nullptr && *app_data != '\0') {
    return app_data;
  }
#else
  if (const char* xdg_config = std::getenv("XDG_CONFIG_HOME");
      xdg_config != nullptr && *xdg_config != '\0') {
    return xdg_config;
  }

  if (const char* home = std::getenv("HOME");
      home != nullptr && *home != '\0') {
    return std::filesystem::path(home) / ".config";
  }
#endif
  // Environment variables are unset: write next to the current directory.
  return GetCurrentDirectory();
}

bool ParseDoubleValue(const std::string& text, double& value) {
  errno = 0;
  char* end = nullptr;
  const double parsed = std::strtod(text.c_str(), &end);
  if (end == text.c_str() || *end != '\0' || errno == ERANGE ||
      !std::isfinite(parsed)) {
    return false;
  }

  value = parsed;
  return true;
}

std::string FormatDouble(double value) {
  char buffer[32];
  std::snprintf(buffer, sizeof(buffer), "%.15g", value);
  return buffer;
}

void ApplySettingLine(const std::string& line, Settings& settings) {
  if (line.starts_with(kLanguageKey)) {
    const std::string value = line.substr(std::string(kLanguageKey).size());
    if (value == kRussianValue) {
      settings.language = Language::kRussian;
    } else if (value == kEnglishValue) {
      settings.language = Language::kEnglish;
    }
  } else if (line.starts_with(kPaletteKey)) {
    const std::string value = line.substr(std::string(kPaletteKey).size());
    if (!value.empty()) {
      settings.palette = value;
    }
  } else if (line.starts_with(kFractalKey)) {
    ParseFractalType(line.substr(std::string(kFractalKey).size()),
                     settings.fractal.type);
  } else if (line.starts_with(kJuliaRealKey)) {
    double value = 0.0;
    if (ParseDoubleValue(line.substr(std::string(kJuliaRealKey).size()),
                         value)) {
      settings.fractal.julia_real = value;
    }
  } else if (line.starts_with(kJuliaImagKey)) {
    double value = 0.0;
    if (ParseDoubleValue(line.substr(std::string(kJuliaImagKey).size()),
                         value)) {
      settings.fractal.julia_imag = value;
    }
  } else if (line.starts_with(kExportPresetKey)) {
    const std::string value = line.substr(std::string(kExportPresetKey).size());
    if (!value.empty()) {
      settings.export_preset = value;
    }
  }
}

}  // namespace

std::filesystem::path GetSettingsPath() {
  return GetConfigRoot() / kConfigDirectory / kSettingsFileName;
}

std::filesystem::path GetLegacySettingsPath() {
  return GetConfigRoot() / kLegacyConfigDirectory / kSettingsFileName;
}

Settings ParseSettings(const std::string& text) {
  Settings settings;
  std::istringstream stream(text);
  std::string line;

  while (std::getline(stream, line)) {
    // ofstream in text mode writes CRLF on Windows.
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    ApplySettingLine(line, settings);
  }

  return settings;
}

std::string FormatSettings(const Settings& settings) {
  const char* language =
      settings.language == Language::kEnglish ? kEnglishValue : kRussianValue;
  std::string text = std::string(kLanguageKey) + language + "\n";
  text += std::string(kPaletteKey) + settings.palette + "\n";
  text += std::string(kFractalKey) + GetFractalTypeName(settings.fractal.type) +
          "\n";
  text += std::string(kJuliaRealKey) +
          FormatDouble(settings.fractal.julia_real) + "\n";
  text += std::string(kJuliaImagKey) +
          FormatDouble(settings.fractal.julia_imag) + "\n";
  text += std::string(kExportPresetKey) + settings.export_preset + "\n";
  return text;
}

Settings LoadSettings(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return Settings{};
  }

  std::ostringstream text;
  text << file.rdbuf();
  return ParseSettings(text.str());
}

Settings LoadSettings(const std::filesystem::path& path,
                      const std::filesystem::path& legacy_path) {
  std::error_code error;
  if (!std::filesystem::exists(path, error) &&
      std::filesystem::exists(legacy_path, error)) {
    const Settings settings = LoadSettings(legacy_path);
    SaveSettings(settings, path);
    return settings;
  }

  return LoadSettings(path);
}

bool SaveSettings(const Settings& settings, const std::filesystem::path& path) {
  const std::filesystem::path directory = path.parent_path();
  if (!directory.empty()) {
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) {
      return false;
    }
  }

  std::ofstream file(path, std::ios::trunc);
  if (!file.is_open()) {
    return false;
  }

  file << FormatSettings(settings);
  file.flush();
  return file.good();
}

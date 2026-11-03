#ifndef FRACTALGENERATION_RENDERER_SETTINGS_HPP_
#define FRACTALGENERATION_RENDERER_SETTINGS_HPP_

#include <filesystem>
#include <string>

#include "fractal_type.hpp"
#include "localization.hpp"

/// Interface settings persisted between runs.
struct Settings {
  Language language = Language::kRussian;
  /// Palette name, Ember by default.
  std::string palette = "Ember";
  /// PNG preset label, FHD by default.
  std::string export_preset = "FHD";
  FractalParams fractal;
};

/// Returns the path to the user settings file.
/// Windows: %APPDATA%\FractalGeneration\settings.txt.
/// Linux: $XDG_CONFIG_HOME/FractalGeneration/settings.txt or
/// ~/.config/FractalGeneration/settings.txt.
std::filesystem::path GetSettingsPath();

/// Path to the settings from before the project rename. It is used to copy
/// settings on the first run.
std::filesystem::path GetLegacySettingsPath();

/// Parses settings file contents. Unknown keys and values are skipped.
Settings ParseSettings(const std::string& text);

/// Writes settings as text with the keys language, palette, fractal,
/// julia-re/im, and png.
std::string FormatSettings(const Settings& settings);

/// Reads settings. If the file is missing, returns the defaults.
Settings LoadSettings(const std::filesystem::path& path);

/// Reads settings. If the file is missing, tries legacy_path and copies it.
Settings LoadSettings(const std::filesystem::path& path,
                      const std::filesystem::path& legacy_path);

/// Writes settings, creating the directory if needed.
/// @return false if the file could not be created or written.
bool SaveSettings(const Settings& settings, const std::filesystem::path& path);

#endif  // FRACTALGENERATION_RENDERER_SETTINGS_HPP_

#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "color_map.hpp"
#include "export_presets.hpp"
#include "fractal_type.hpp"
#include "localization.hpp"
#include "settings.hpp"

namespace {

TEST(SettingsTest, DefaultsToRussian) {
  EXPECT_EQ(ParseSettings("").language, Language::kRussian);
}

TEST(SettingsTest, ParsesEnglish) {
  EXPECT_EQ(ParseSettings("language=en\n").language, Language::kEnglish);
}

TEST(SettingsTest, ParsesRussian) {
  EXPECT_EQ(ParseSettings("language=ru\n").language, Language::kRussian);
}

TEST(SettingsTest, IgnoresUnknownKeysAndValues) {
  const Settings settings =
      ParseSettings("palette=Ocean\nlanguage=de\nlanguage=en\n");

  EXPECT_EQ(settings.language, Language::kEnglish);
}

TEST(SettingsTest, ParsesWindowsLineEndings) {
  EXPECT_EQ(ParseSettings("language=en\r\n").language, Language::kEnglish);
}

TEST(SettingsTest, ParsesPalette) {
  EXPECT_EQ(ParseSettings("palette=Ocean\n").palette, "Ocean");
}

TEST(SettingsTest, DefaultPaletteIsKnown) {
  ColorMap colormap;
  EXPECT_TRUE(colormap.SetPalette(Settings{}.palette));
}

TEST(SettingsTest, ParsesExportPreset) {
  EXPECT_EQ(ParseSettings("png=4K\n").export_preset, "4K");
}

TEST(SettingsTest, ParsesFractalSettings) {
  const Settings settings =
      ParseSettings("fractal=julia\njulia-re=-0.4\njulia-im=0.6\n");

  EXPECT_EQ(settings.fractal.type, FractalType::kJulia);
  EXPECT_DOUBLE_EQ(settings.fractal.julia_real, -0.4);
  EXPECT_DOUBLE_EQ(settings.fractal.julia_imag, 0.6);
}

TEST(SettingsTest, ParsesBurningShip) {
  EXPECT_EQ(ParseSettings("fractal=burningship\n").fractal.type,
            FractalType::kBurningShip);
}

TEST(SettingsTest, DefaultsToMandelbrotParameters) {
  const Settings settings;

  EXPECT_EQ(settings.fractal.type, FractalType::kMandelbrot);
  EXPECT_DOUBLE_EQ(settings.fractal.julia_real, -0.8);
  EXPECT_DOUBLE_EQ(settings.fractal.julia_imag, 0.156);
}

TEST(SettingsTest, IgnoresInvalidJuliaValues) {
  const Settings settings =
      ParseSettings("julia-re=abc\njulia-im=nan\nfractal=unknown\n");

  EXPECT_EQ(settings.fractal.type, FractalType::kMandelbrot);
  EXPECT_DOUBLE_EQ(settings.fractal.julia_real, -0.8);
  EXPECT_DOUBLE_EQ(settings.fractal.julia_imag, 0.156);
}

TEST(SettingsTest, DefaultExportPresetIsKnown) {
  EXPECT_EQ(GetExportPresetIndex(Settings{}.export_preset), 0);
}

TEST(SettingsTest, FormatRoundTrip) {
  Settings settings;
  settings.language = Language::kEnglish;
  settings.palette = "Ocean";
  settings.fractal.type = FractalType::kJulia;
  settings.fractal.julia_real = -0.4;
  settings.fractal.julia_imag = 0.6;
  settings.export_preset = "8K";

  EXPECT_EQ(FormatSettings(settings),
            "language=en\npalette=Ocean\nfractal=julia\njulia-re=-0.4\n"
            "julia-im=0.6\npng=8K\n");
  const Settings parsed = ParseSettings(FormatSettings(settings));
  EXPECT_EQ(parsed.language, Language::kEnglish);
  EXPECT_EQ(parsed.palette, "Ocean");
  EXPECT_EQ(parsed.fractal.type, FractalType::kJulia);
  EXPECT_DOUBLE_EQ(parsed.fractal.julia_real, -0.4);
  EXPECT_DOUBLE_EQ(parsed.fractal.julia_imag, 0.6);
  EXPECT_EQ(parsed.export_preset, "8K");
}

TEST(SettingsTest, PathPointsToSettingsFile) {
  const std::filesystem::path path = GetSettingsPath();

  EXPECT_EQ(path.filename().string(), "settings.txt");
  EXPECT_EQ(path.parent_path().filename().string(), "FractalGeneration");
}

TEST(SettingsTest, LegacySettingsPathPointsToOldDirectory) {
  const std::filesystem::path path = GetLegacySettingsPath();

  EXPECT_EQ(path.filename().string(), "settings.txt");
  EXPECT_EQ(path.parent_path().filename().string(),
            "MandelbrotFractalGeneration");
}

TEST(SettingsTest, SaveAndLoadFile) {
  const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                     "mandelbrot_settings_save_test.txt";
  std::filesystem::remove(path);

  Settings settings;
  settings.language = Language::kEnglish;
  settings.palette = "Ocean";
  settings.fractal.type = FractalType::kJulia;
  settings.fractal.julia_real = -0.4;
  settings.fractal.julia_imag = 0.6;
  settings.export_preset = "8K";

  ASSERT_TRUE(SaveSettings(settings, path));
  const Settings loaded = LoadSettings(path);
  EXPECT_EQ(loaded.language, Language::kEnglish);
  EXPECT_EQ(loaded.palette, "Ocean");
  EXPECT_EQ(loaded.fractal.type, FractalType::kJulia);
  EXPECT_DOUBLE_EQ(loaded.fractal.julia_real, -0.4);
  EXPECT_DOUBLE_EQ(loaded.fractal.julia_imag, 0.6);
  EXPECT_EQ(loaded.export_preset, "8K");

  std::filesystem::remove(path);
}

TEST(SettingsTest, MissingFileKeepsDefaults) {
  const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                     "mandelbrot_settings_missing_test.txt";
  std::filesystem::remove(path);

  EXPECT_EQ(LoadSettings(path).language, Language::kRussian);
}

TEST(SettingsTest, SaveCreatesMissingDirectory) {
  const std::filesystem::path root = std::filesystem::temp_directory_path() /
                                     "mandelbrot_settings_directory_test";
  std::filesystem::remove_all(root);
  const std::filesystem::path path = root / "nested" / "settings.txt";

  ASSERT_TRUE(SaveSettings(Settings{}, path));
  EXPECT_TRUE(std::filesystem::exists(path));

  std::filesystem::remove_all(root);
}

TEST(SettingsTest, MigratesLegacyFile) {
  const std::filesystem::path root = std::filesystem::temp_directory_path() /
                                     "fractal_settings_migration_test";
  std::filesystem::remove_all(root);
  const std::filesystem::path legacy = root / "legacy" / "settings.txt";
  const std::filesystem::path current = root / "current" / "settings.txt";

  Settings old;
  old.language = Language::kEnglish;
  old.palette = "Ocean";
  old.fractal.type = FractalType::kJulia;
  old.fractal.julia_real = -0.4;
  old.fractal.julia_imag = 0.6;
  old.export_preset = "8K";
  ASSERT_TRUE(SaveSettings(old, legacy));

  const Settings migrated = LoadSettings(current, legacy);
  EXPECT_EQ(migrated.language, Language::kEnglish);
  EXPECT_EQ(migrated.palette, "Ocean");
  EXPECT_EQ(migrated.fractal.type, FractalType::kJulia);
  EXPECT_DOUBLE_EQ(migrated.fractal.julia_real, -0.4);
  EXPECT_DOUBLE_EQ(migrated.fractal.julia_imag, 0.6);
  EXPECT_EQ(migrated.export_preset, "8K");
  EXPECT_TRUE(std::filesystem::exists(current));

  std::filesystem::remove_all(root);
}

TEST(SettingsTest, CurrentFileWinsOverLegacy) {
  const std::filesystem::path root =
      std::filesystem::temp_directory_path() / "fractal_settings_priority_test";
  std::filesystem::remove_all(root);
  const std::filesystem::path legacy = root / "legacy" / "settings.txt";
  const std::filesystem::path current = root / "current" / "settings.txt";

  Settings old;
  old.language = Language::kEnglish;
  Settings fresh;
  fresh.language = Language::kRussian;
  ASSERT_TRUE(SaveSettings(old, legacy));
  ASSERT_TRUE(SaveSettings(fresh, current));

  EXPECT_EQ(LoadSettings(current, legacy).language, Language::kRussian);

  std::filesystem::remove_all(root);
}

}  // namespace

#include "ColorMap.h"
#include "ExportPresets.h"
#include "Settings.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

namespace {

	TEST(SettingsTest, DefaultsToRussian) {
		EXPECT_EQ(parseSettings("").language, Language::Russian);
	}

	TEST(SettingsTest, ParsesEnglish) {
		EXPECT_EQ(parseSettings("language=en\n").language, Language::English);
	}

	TEST(SettingsTest, ParsesRussian) {
		EXPECT_EQ(parseSettings("language=ru\n").language, Language::Russian);
	}

	TEST(SettingsTest, IgnoresUnknownKeysAndValues) {
		const Settings settings = parseSettings("palette=Ocean\nlanguage=de\nlanguage=en\n");

		EXPECT_EQ(settings.language, Language::English);
	}

	TEST(SettingsTest, ParsesWindowsLineEndings) {
		EXPECT_EQ(parseSettings("language=en\r\n").language, Language::English);
	}

	TEST(SettingsTest, ParsesPalette) {
		EXPECT_EQ(parseSettings("palette=Ocean\n").palette, "Ocean");
	}

	TEST(SettingsTest, DefaultPaletteIsKnown) {
		ColorMap colormap;
		EXPECT_TRUE(colormap.setPalette(Settings{}.palette));
	}

	TEST(SettingsTest, ParsesExportPreset) {
		EXPECT_EQ(parseSettings("png=4K\n").exportPreset, "4K");
	}

	TEST(SettingsTest, DefaultExportPresetIsKnown) {
		EXPECT_EQ(exportPresetIndex(Settings{}.exportPreset), 0);
	}

	TEST(SettingsTest, FormatRoundTrip) {
		Settings settings;
		settings.language = Language::English;
		settings.palette = "Ocean";
		settings.exportPreset = "8K";

		EXPECT_EQ(formatSettings(settings), "language=en\npalette=Ocean\npng=8K\n");
		const Settings parsed = parseSettings(formatSettings(settings));
		EXPECT_EQ(parsed.language, Language::English);
		EXPECT_EQ(parsed.palette, "Ocean");
		EXPECT_EQ(parsed.exportPreset, "8K");
	}

	TEST(SettingsTest, PathPointsToSettingsFile) {
		const std::filesystem::path path = settingsPath();

		EXPECT_EQ(path.filename().string(), "settings.txt");
		EXPECT_EQ(path.parent_path().filename().string(), "MandelbrotFractalGeneration");
	}

	TEST(SettingsTest, SaveAndLoadFile) {
		const std::filesystem::path path = std::filesystem::temp_directory_path() / "mandelbrot_settings_save_test.txt";
		std::filesystem::remove(path);

		Settings settings;
		settings.language = Language::English;
		settings.palette = "Ocean";
		settings.exportPreset = "8K";

		ASSERT_TRUE(saveSettings(settings, path));
		const Settings loaded = loadSettings(path);
		EXPECT_EQ(loaded.language, Language::English);
		EXPECT_EQ(loaded.palette, "Ocean");
		EXPECT_EQ(loaded.exportPreset, "8K");

		std::filesystem::remove(path);
	}

	TEST(SettingsTest, MissingFileKeepsDefaults) {
		const std::filesystem::path path =
			std::filesystem::temp_directory_path() / "mandelbrot_settings_missing_test.txt";
		std::filesystem::remove(path);

		EXPECT_EQ(loadSettings(path).language, Language::Russian);
	}

	TEST(SettingsTest, SaveCreatesMissingDirectory) {
		const std::filesystem::path root =
			std::filesystem::temp_directory_path() / "mandelbrot_settings_directory_test";
		std::filesystem::remove_all(root);
		const std::filesystem::path path = root / "nested" / "settings.txt";

		ASSERT_TRUE(saveSettings(Settings{}, path));
		EXPECT_TRUE(std::filesystem::exists(path));

		std::filesystem::remove_all(root);
	}

} // namespace

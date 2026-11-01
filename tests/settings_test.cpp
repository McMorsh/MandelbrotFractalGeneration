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

	TEST(SettingsTest, FormatRoundTrip) {
		Settings settings;
		settings.language = Language::English;

		EXPECT_EQ(formatSettings(settings), "language=en\n");
		EXPECT_EQ(parseSettings(formatSettings(settings)).language, Language::English);
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

		ASSERT_TRUE(saveSettings(settings, path));
		EXPECT_EQ(loadSettings(path).language, Language::English);

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

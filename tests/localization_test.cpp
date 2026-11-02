#include "Localization.h"

#include <gtest/gtest.h>

#include <string>

namespace {

	TEST(LocalizationTest, NextLanguageToggles) {
		EXPECT_EQ(nextLanguage(Language::Russian), Language::English);
		EXPECT_EQ(nextLanguage(Language::English), Language::Russian);
	}

	TEST(LocalizationTest, LanguagesHaveDifferentText) {
		const UiText& russian = uiText(Language::Russian);
		const UiText& english = uiText(Language::English);

		EXPECT_STRNE(russian.copied, english.copied);
		EXPECT_STRNE(russian.statusRendering, english.statusRendering);
	}

	TEST(LocalizationTest, FractalTitlesAreLocalized) {
		EXPECT_STRNE(fractalTitle(FractalType::Mandelbrot, Language::Russian),
					 fractalTitle(FractalType::Mandelbrot, Language::English));
		EXPECT_STRNE(fractalTitle(FractalType::Julia, Language::Russian),
					 fractalTitle(FractalType::Julia, Language::English));
		EXPECT_STRNE(fractalTitle(FractalType::Mandelbrot, Language::Russian),
					 fractalTitle(FractalType::Julia, Language::Russian));
		EXPECT_STRNE(fractalTitle(FractalType::BurningShip, Language::Russian),
					 fractalTitle(FractalType::BurningShip, Language::English));
		EXPECT_STRNE(fractalTitle(FractalType::Mandelbrot, Language::Russian),
					 fractalTitle(FractalType::BurningShip, Language::Russian));
	}

	TEST(LocalizationTest, HelpLinesAreComplete) {
		for (const Language language : {Language::Russian, Language::English}) {
			const HelpLineText* lines = helpLines(language);
			ASSERT_NE(lines, nullptr);

			for (int i = 0; i < kHelpLineCount; ++i) {
				EXPECT_NE(lines[i].keys, nullptr);
				EXPECT_NE(lines[i].action, nullptr);
			}
		}
	}

	TEST(LocalizationTest, LanguageSwitchIsListedInHelp) {
		const HelpLineText* lines = helpLines(Language::Russian);

		bool found = false;
		for (int i = 0; i < kHelpLineCount; ++i) {
			if (std::string(lines[i].keys) == "L")
				found = true;
		}

		EXPECT_TRUE(found);
	}

	TEST(LocalizationTest, FractalSwitchIsListedInHelp) {
		const HelpLineText* lines = helpLines(Language::Russian);

		bool found = false;
		for (int i = 0; i < kHelpLineCount; ++i) {
			if (std::string(lines[i].keys) == "T")
				found = true;
		}

		EXPECT_TRUE(found);
	}

} // namespace

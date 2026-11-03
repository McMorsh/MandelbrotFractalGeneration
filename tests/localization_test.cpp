#include <string>

#include <gtest/gtest.h>

#include "fractal_type.hpp"
#include "localization.hpp"

namespace {

TEST(LocalizationTest, NextLanguageToggles) {
  EXPECT_EQ(GetNextLanguage(Language::kRussian), Language::kEnglish);
  EXPECT_EQ(GetNextLanguage(Language::kEnglish), Language::kRussian);
}

TEST(LocalizationTest, LanguagesHaveDifferentText) {
  const UiText& russian = GetUiText(Language::kRussian);
  const UiText& english = GetUiText(Language::kEnglish);

  EXPECT_STRNE(russian.copied, english.copied);
  EXPECT_STRNE(russian.status_rendering, english.status_rendering);
}

TEST(LocalizationTest, FractalTitlesAreLocalized) {
  EXPECT_STRNE(GetFractalTitle(FractalType::kMandelbrot, Language::kRussian),
               GetFractalTitle(FractalType::kMandelbrot, Language::kEnglish));
  EXPECT_STRNE(GetFractalTitle(FractalType::kJulia, Language::kRussian),
               GetFractalTitle(FractalType::kJulia, Language::kEnglish));
  EXPECT_STRNE(GetFractalTitle(FractalType::kMandelbrot, Language::kRussian),
               GetFractalTitle(FractalType::kJulia, Language::kRussian));
  EXPECT_STRNE(GetFractalTitle(FractalType::kBurningShip, Language::kRussian),
               GetFractalTitle(FractalType::kBurningShip, Language::kEnglish));
  EXPECT_STRNE(GetFractalTitle(FractalType::kMandelbrot, Language::kRussian),
               GetFractalTitle(FractalType::kBurningShip, Language::kRussian));
}

TEST(LocalizationTest, HelpLinesAreComplete) {
  for (const Language language : {Language::kRussian, Language::kEnglish}) {
    const HelpLineText* lines = GetHelpLines(language);
    ASSERT_NE(lines, nullptr);

    for (int i = 0; i < kHelpLineCount; ++i) {
      EXPECT_NE(lines[i].keys, nullptr);
      EXPECT_NE(lines[i].action, nullptr);
    }
  }
}

TEST(LocalizationTest, LanguageSwitchIsListedInHelp) {
  const HelpLineText* lines = GetHelpLines(Language::kRussian);

  bool found = false;
  for (int i = 0; i < kHelpLineCount; ++i) {
    if (std::string(lines[i].keys) == "L") {
      found = true;
    }
  }

  EXPECT_TRUE(found);
}

TEST(LocalizationTest, FractalSwitchIsListedInHelp) {
  const HelpLineText* lines = GetHelpLines(Language::kRussian);

  bool found = false;
  for (int i = 0; i < kHelpLineCount; ++i) {
    if (std::string(lines[i].keys) == "T") {
      found = true;
    }
  }

  EXPECT_TRUE(found);
}

}  // namespace

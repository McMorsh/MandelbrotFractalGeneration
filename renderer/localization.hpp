#ifndef FRACTALGENERATION_RENDERER_LOCALIZATION_HPP_
#define FRACTALGENERATION_RENDERER_LOCALIZATION_HPP_

#include "fractal_type.hpp"

/// Interface language.
enum class Language {
  kRussian,
  kEnglish,
};

/// Fixed interface strings. Format strings are intended for snprintf.
struct UiText {
  const char* controls_header;
  const char* info_format;
  const char* details_format;
  const char* details_narrow_format;
  const char* image_age_format;
  const char* copied;
  const char* clipboard_no_view;
  const char* unknown_palette;
  const char* cannot_restore_iterations;
  const char* pasted_view;
  const char* save_in_progress;
  const char* rendering_export_format;
  const char* saved_prefix;
  const char* save_failed_prefix;
  const char* status_rendering;
};

/// Help panel line: keys on the left, action on the right.
struct HelpLineText {
  const char* keys;
  const char* action;
};

/// Number of help panel lines.
inline constexpr int kHelpLineCount = 12;

/// Fractal name for the window title and HUD.
const char* GetFractalTitle(FractalType type, Language language);

/// Returns the opposite language.
Language GetNextLanguage(Language language);

/// Returns the strings for the selected language.
const UiText& GetUiText(Language language);

/// Returns the help panel lines for the selected language.
const HelpLineText* GetHelpLines(Language language);

#endif  // FRACTALGENERATION_RENDERER_LOCALIZATION_HPP_

#include "localization.hpp"

#include <iterator>

#include "fractal_type.hpp"

namespace {

constexpr UiText kRussianText = {
    "УПРАВЛЕНИЕ",
    "масштаб %.3gx   |   итераций %d   |   %s   | рендер %.1f мс   |   PNG %s",
    "центр (%.15g, %+.15gi)   |   курсор (%.15g, %+.15gi)",
    "курсор (%.15g, %+.15gi)",
    "   |   время с рендера %.0f мс",
    "Скопировано в буфер обмена",
    "В буфере обмена нет параметров вида",
    "Неизвестная палитра в буфере обмена",
    "Не удалось восстановить итерации",
    "Вид вставлен",
    "Сохранение уже идёт",
    "Рендер %s...",
    "Сохранено ",
    "Не удалось сохранить ",
    "рендер",
};

constexpr UiText kEnglishText = {
    "CONTROLS",
    "zoom %.3gx   |   iterations %d   |   %s   |   render %.1f ms   |   PNG %s",
    "center (%.15g, %+.15gi)   |   cursor (%.15g, %+.15gi)",
    "cursor (%.15g, %+.15gi)",
    "   |   image age %.0f ms",
    "Copied to clipboard",
    "Clipboard has no view data",
    "Unknown palette in clipboard",
    "Cannot restore iterations",
    "Pasted view",
    "Save already in progress",
    "Rendering %s...",
    "Saved ",
    "Could not save ",
    "rendering",
};

constexpr HelpLineText kRussianHelp[] = {
    {"Колесо, + / -", "управление масштабом"},
    {"Стрелки, ЛКМ", "панорама"},
    {"[ / ]", "итерации"},
    {"P", "палитра"},
    {"C / V", "копировать / вставить"},
    {"F2", "сохранить PNG"},
    {"Shift + влево / вправо", "размер PNG"},
    {"T", "тип фрактала"},
    {"R", "сброс вида"},
    {"L", "локализация"},
    {"H", "справка"},
    {"Esc", "выход"},
};

constexpr HelpLineText kEnglishHelp[] = {
    {"Wheel, + / -", "zoom"},
    {"Arrows, LMB", "pan"},
    {"[ / ]", "iterations"},
    {"P", "palette"},
    {"C / V", "copy / paste"},
    {"F2", "save PNG"},
    {"Shift + Left / Right", "PNG size"},
    {"T", "fractal type"},
    {"R", "reset view"},
    {"L", "language"},
    {"H", "help"},
    {"Esc", "quit"},
};

static_assert(std::size(kRussianHelp) == kHelpLineCount);
static_assert(std::size(kEnglishHelp) == kHelpLineCount);

}  // namespace

Language GetNextLanguage(Language language) {
  return language == Language::kRussian ? Language::kEnglish
                                        : Language::kRussian;
}

const UiText& GetUiText(Language language) {
  return language == Language::kRussian ? kRussianText : kEnglishText;
}

const HelpLineText* GetHelpLines(Language language) {
  return language == Language::kRussian ? kRussianHelp : kEnglishHelp;
}

const char* GetFractalTitle(FractalType type, Language language) {
  if (language == Language::kRussian) {
    switch (type) {
      case FractalType::kMandelbrot:
        return "Множество Мандельброта";
      case FractalType::kJulia:
        return "Множество Жюлиа";
      case FractalType::kBurningShip:
        return "Горящий корабль";
    }
    return "Фрактал";
  }

  switch (type) {
    case FractalType::kMandelbrot:
      return "Mandelbrot Set";
    case FractalType::kJulia:
      return "Julia Set";
    case FractalType::kBurningShip:
      return "Burning Ship";
  }
  return "Fractal";
}

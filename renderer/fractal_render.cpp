#include "fractal_render.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include <raylib.h>

namespace {

constexpr int kMinWindowWidth = 640;
constexpr int kMinWindowHeight = 480;
constexpr int kTargetFps = 60;
constexpr int kTopBarHeight = 96;
constexpr int kTopBarTextX = 24;
constexpr int kTopBarTitleY = 14;
constexpr int kTopBarInfoY = 46;
constexpr int kTopBarDetailsY = 68;
constexpr int kTopBarFpsY = 22;
constexpr int kTopBarHelpGap = 12;
constexpr int kTextRightMargin = 24;
constexpr int kTextLimitPadding = 48;
constexpr int kPanelWidth = 380;
constexpr int kPanelPadding = 16;
constexpr int kHelpHeaderHeight = 28;
constexpr int kHelpPanelMargin = 24;
constexpr int kLineHeight = 20;
constexpr int kHelpKeyColumnWidth = 170;
constexpr int kFontAtlasSize = 32;
constexpr int kTitleFontSize = 24;
constexpr int kHudFontSize = 16;
constexpr float kTextSpacing = 0.0f;
constexpr int kJuliaPanelWidth = 340;
constexpr int kJuliaPanelHeight = 104;
constexpr int kJuliaPanelMargin = 24;
constexpr int kJuliaPanelLabelOffsetX = 14;
constexpr int kJuliaPanelLabelOffsetY = 12;
constexpr int kJuliaTrackHeight = 6;
constexpr int kJuliaTrackOffsetX = 52;
constexpr int kJuliaTrackWidth = 200;
constexpr int kJuliaRealTrackOffsetY = 42;
constexpr int kJuliaImagTrackOffsetY = 70;
constexpr int kJuliaLabelOffsetX = 38;
constexpr int kJuliaLabelOffsetY = 6;
constexpr int kJuliaValueOffsetX = 12;
constexpr int kJuliaValueOffsetY = 6;
constexpr float kJuliaSliderHitPaddingX = 8.0f;
constexpr float kJuliaSliderHitPaddingY = 12.0f;
constexpr int kAsciiFirstCode = 32;
constexpr int kAsciiLastCode = 126;
constexpr int kCyrillicFirstCode = 0x0400;
constexpr int kCyrillicLastCode = 0x04FF;
constexpr float kOverlayAlpha = 0.55f;
constexpr float kTrackAlpha = 0.22f;
constexpr float kInfoAlpha = 0.72f;
constexpr float kNoticeAlpha = 0.85f;
constexpr float kFpsAlpha = 0.6f;
constexpr float kHelpActionAlpha = 0.75f;
constexpr float kSliderValueAlpha = 0.8f;
constexpr float kJuliaActiveHandleRadius = 8.0f;
constexpr float kJuliaInactiveHandleRadius = 6.0f;
constexpr double kJuliaMin = -2.0;
constexpr double kJuliaMax = 2.0;

constexpr int kPanelHeight =
    kPanelPadding * 2 + kHelpHeaderHeight + kHelpLineCount * kLineHeight;

std::string GetAssetPath(const char* name) {
  std::string directory = GetApplicationDirectory();
  if (!directory.empty() && directory.back() != '/' &&
      directory.back() != '\\') {
    directory += '/';
  }
  return directory + "assets/" + name;
}

Font LoadInterfaceFont(bool& custom_font) {
  custom_font = false;

  // ASCII and Cyrillic; the range covers Russian letters and "yo".
  std::vector<int> codepoints;
  codepoints.reserve((kAsciiLastCode - kAsciiFirstCode + 1) +
                     (kCyrillicLastCode - kCyrillicFirstCode + 1));
  for (int code = kAsciiFirstCode; code <= kAsciiLastCode; ++code) {
    codepoints.push_back(code);
  }
  for (int code = kCyrillicFirstCode; code <= kCyrillicLastCode; ++code) {
    codepoints.push_back(code);
  }

  const std::string path = GetAssetPath("IBMPlexMono-Regular.ttf");
  Font loaded = LoadFontEx(path.c_str(), kFontAtlasSize, codepoints.data(),
                           static_cast<int>(codepoints.size()));
  if (loaded.texture.id == 0) {
    TraceLog(LOG_WARNING, "Font not found: %s", path.c_str());
    return GetFontDefault();
  }

  custom_font = true;
  SetTextureFilter(loaded.texture, TEXTURE_FILTER_BILINEAR);
  return loaded;
}

void DrawUiText(const Font& font, const char* text, int x, int y, int font_size,
                Color color) {
  DrawTextEx(font, text, Vector2{static_cast<float>(x), static_cast<float>(y)},
             static_cast<float>(font_size), kTextSpacing, color);
}

int MeasureUiText(const Font& font, const char* text, int font_size) {
  return static_cast<int>(
      MeasureTextEx(font, text, static_cast<float>(font_size), kTextSpacing).x);
}

std::size_t GetUtf8Length(const std::string& text) {
  std::size_t length = 0;
  for (const unsigned char byte : text) {
    if ((byte & 0xC0) != 0x80) {
      ++length;
    }
  }
  return length;
}

std::size_t GetUtf8Offset(const std::string& text,
                          std::size_t code_point_index) {
  std::size_t index = 0;
  for (std::size_t i = 0; i < text.size(); ++i) {
    if ((static_cast<unsigned char>(text[i]) & 0xC0) != 0x80) {
      if (index == code_point_index) {
        return i;
      }
      ++index;
    }
  }
  return text.size();
}

// Truncates text in the middle without breaking UTF-8 characters.
std::string FitText(const Font& font, const std::string& text, int max_width,
                    int font_size) {
  if (MeasureUiText(font, text.c_str(), font_size) <= max_width) {
    return text;
  }

  constexpr const char* kDots = "...";
  const std::size_t length = GetUtf8Length(text);
  std::size_t keep_left = length / 2;
  std::size_t keep_right = length - keep_left;

  while (keep_left > 0 && keep_right > 0) {
    const std::string candidate =
        text.substr(0, GetUtf8Offset(text, keep_left)) + kDots +
        text.substr(GetUtf8Offset(text, length - keep_right));
    if (MeasureUiText(font, candidate.c_str(), font_size) <= max_width) {
      return candidate;
    }

    if (keep_left > keep_right) {
      --keep_left;
    } else {
      --keep_right;
    }
  }

  return kDots;
}

struct JuliaLayout {
  Rectangle panel;
  Rectangle real_track;
  Rectangle imag_track;
};

JuliaLayout MakeJuliaLayout(int width, int height) {
  const float panel_x =
      static_cast<float>(width) - kJuliaPanelWidth - kJuliaPanelMargin;
  const float panel_y =
      static_cast<float>(height) - kJuliaPanelHeight - kJuliaPanelMargin;
  const float track_x = panel_x + kJuliaTrackOffsetX;
  const float track_width = kJuliaTrackWidth;
  return {
      {panel_x, panel_y, static_cast<float>(kJuliaPanelWidth),
       static_cast<float>(kJuliaPanelHeight)},
      {track_x, panel_y + kJuliaRealTrackOffsetY, track_width,
       static_cast<float>(kJuliaTrackHeight)},
      {track_x, panel_y + kJuliaImagTrackOffsetY, track_width,
       static_cast<float>(kJuliaTrackHeight)},
  };
}

void DrawJuliaSlider(const Font& font, const Rectangle& track,
                     const char* label, double value, bool active) {
  const double t =
      std::clamp((value - kJuliaMin) / (kJuliaMax - kJuliaMin), 0.0, 1.0);
  const float handle_x = track.x + static_cast<float>(t) * track.width;
  const float handle_y = track.y + track.height * 0.5f;

  DrawUiText(font, label, static_cast<int>(track.x) - kJuliaLabelOffsetX,
             static_cast<int>(track.y) - kJuliaLabelOffsetY, kHudFontSize,
             RAYWHITE);
  DrawRectangleRec(track, Fade(RAYWHITE, kTrackAlpha));
  DrawRectangle(static_cast<int>(track.x), static_cast<int>(track.y),
                static_cast<int>(handle_x - track.x),
                static_cast<int>(track.height), Fade(RAYWHITE, kOverlayAlpha));
  DrawCircle(static_cast<int>(handle_x), static_cast<int>(handle_y),
             active ? kJuliaActiveHandleRadius : kJuliaInactiveHandleRadius,
             active ? ORANGE : RAYWHITE);
  DrawTextEx(font, TextFormat("%.3f", value),
             Vector2{track.x + track.width + kJuliaValueOffsetX,
                     track.y - kJuliaValueOffsetY},
             static_cast<float>(kHudFontSize), kTextSpacing,
             Fade(RAYWHITE, kSliderValueAlpha));
}

void DrawJuliaPanel(const Font& font, int width, int height,
                    const JuliaControls& controls,
                    const FractalParams& fractal) {
  const JuliaLayout layout = MakeJuliaLayout(width, height);
  DrawRectangleRec(layout.panel, Fade(BLACK, kOverlayAlpha));
  DrawUiText(font, "Julia c",
             static_cast<int>(layout.panel.x) + kJuliaPanelLabelOffsetX,
             static_cast<int>(layout.panel.y) + kJuliaPanelLabelOffsetY,
             kHudFontSize, Fade(RAYWHITE, kOverlayAlpha));
  DrawJuliaSlider(font, layout.real_track, "Re", fractal.julia_real,
                  controls.active_slider == 0);
  DrawJuliaSlider(font, layout.imag_track, "Im", fractal.julia_imag,
                  controls.active_slider == 1);
}

void DrawTopBar(const Font& font, int width, const std::string& info,
                const std::string& details, const std::string& notice,
                const std::string& status, Language language,
                FractalType fractal_type) {
  DrawRectangle(0, 0, width, kTopBarHeight, Fade(BLACK, kOverlayAlpha));
  DrawUiText(font, GetFractalTitle(fractal_type, language), kTopBarTextX,
             kTopBarTitleY, kTitleFontSize, RAYWHITE);

  // Long lines are not drawn in a narrow window: they overflow or overlap FPS.
  const int text_limit = width - kTextLimitPadding;
  if (MeasureUiText(font, info.c_str(), kHudFontSize) <= text_limit) {
    DrawUiText(font, info.c_str(), kTopBarTextX, kTopBarInfoY, kHudFontSize,
               Fade(RAYWHITE, kInfoAlpha));
  }

  bool notice_shown = false;
  if (!notice.empty()) {
    // A notice is more important than coordinates: truncate it but do not
    // hide it.
    const std::string notice_text =
        FitText(font, notice, text_limit, kHudFontSize);
    DrawUiText(font, notice_text.c_str(), kTopBarTextX, kTopBarDetailsY,
               kHudFontSize, Fade(RAYWHITE, kNoticeAlpha));
    notice_shown = true;
  }
  if (!notice_shown &&
      MeasureUiText(font, details.c_str(), kHudFontSize) <= text_limit) {
    DrawUiText(font, details.c_str(), kTopBarTextX, kTopBarDetailsY,
               kHudFontSize, Fade(RAYWHITE, kOverlayAlpha));
  }

  const char* fps =
      status.empty() ? TextFormat("%d FPS", GetFPS())
                     : TextFormat("%d FPS   |   %s", GetFPS(), status.c_str());
  const int fps_width = MeasureUiText(font, fps, kHudFontSize);
  DrawUiText(font, fps, width - fps_width - kTextRightMargin, kTopBarFpsY,
             kHudFontSize, Fade(RAYWHITE, kFpsAlpha));
}

void DrawHelpPanel(const Font& font, int height, Language language) {
  // With a small height the panel moves close to the top line.
  const int panel_x = kHelpPanelMargin;
  const int panel_y = std::max(height - kPanelHeight - kHelpPanelMargin,
                               kTopBarHeight + kTopBarHelpGap);

  DrawRectangle(panel_x, panel_y, kPanelWidth, kPanelHeight,
                Fade(BLACK, kOverlayAlpha));
  const UiText& text = GetUiText(language);
  DrawUiText(font, text.controls_header, panel_x + kPanelPadding,
             panel_y + kPanelPadding, kHudFontSize,
             Fade(RAYWHITE, kOverlayAlpha));

  const HelpLineText* lines = GetHelpLines(language);
  int line_y = panel_y + kPanelPadding + kHelpHeaderHeight;
  const int key_x = panel_x + kPanelPadding;
  const int action_x = key_x + kHelpKeyColumnWidth;
  for (int i = 0; i < kHelpLineCount; ++i) {
    DrawUiText(font, lines[i].keys, key_x, line_y, kHudFontSize, RAYWHITE);
    DrawUiText(font, lines[i].action, action_x, line_y, kHudFontSize,
               Fade(RAYWHITE, kHelpActionAlpha));
    line_y += kLineHeight;
  }
}

}  // namespace

FractalRender::FractalRender(int width, int height, const char* title) {
  InitWindow(width, height, title);
  SetTargetFPS(kTargetFps);
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetWindowMinSize(kMinWindowWidth, kMinWindowHeight);

  font_ = LoadInterfaceFont(custom_font_);

  // Use the actual size: the OS may adjust the requested one.
  width_ = GetScreenWidth();
  height_ = GetScreenHeight();

  image_ = GenImageColor(width_, height_, BLACK);
  texture_ = LoadTextureFromImage(image_);
  // Without smoothing, fractal pixels stay sharp.
  SetTextureFilter(texture_, TEXTURE_FILTER_POINT);
}

FractalRender::~FractalRender() {
  UnloadTexture(texture_);
  UnloadImage(image_);
  if (custom_font_) {
    UnloadFont(font_);
  }
  CloseWindow();
}

void FractalRender::Resize(int width, int height) {
  if (width <= 0 || height <= 0) {
    return;
  }

  width_ = width;
  height_ = height;
}

void FractalRender::Draw(const std::vector<double>& data,
                         const ColorMap& colormap, int max_iter, int data_width,
                         int data_height) {
  if (data_width <= 0 || data_height <= 0) {
    return;
  }
  if (data.size() != static_cast<std::size_t>(data_width) * data_height) {
    return;
  }

  // Preview and full frames have different sizes, so the texture is recreated
  // only when the resolution changes.
  if (image_.width != data_width || image_.height != data_height) {
    UnloadTexture(texture_);
    UnloadImage(image_);

    image_ = GenImageColor(data_width, data_height, BLACK);
    texture_ = LoadTextureFromImage(image_);
    SetTextureFilter(texture_, TEXTURE_FILTER_POINT);
  }

  // GenImageColor creates an RGBA image, so pixels can be written directly.
  auto* pixels = static_cast<Color*>(image_.data);

#pragma omp parallel for schedule(static)
  for (int y = 0; y < data_height; y++) {
    for (int x = 0; x < data_width; x++) {
      const std::size_t idx = static_cast<std::size_t>(y) * data_width + x;
      pixels[idx] = colormap.GetColor(data[idx], max_iter);
    }
  }

  UpdateTexture(texture_, image_.data);
}

void FractalRender::Display(const std::string& info, const std::string& details,
                            const std::string& notice,
                            const std::string& status, bool show_help,
                            const JuliaControls& julia_controls,
                            const FractalParams& fractal) const {
  BeginDrawing();
  ClearBackground(BLACK);

  // The frame may be smaller than the window when a preview is shown.
  const Rectangle source = {0.0f, 0.0f, static_cast<float>(image_.width),
                            static_cast<float>(image_.height)};
  const Rectangle dest = {0.0f, 0.0f, static_cast<float>(width_),
                          static_cast<float>(height_)};
  DrawTexturePro(texture_, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);

  DrawTopBar(font_, width_, info, details, notice, status, language_,
             fractal_type_);

  if (show_help) {
    DrawHelpPanel(font_, height_, language_);
  }

  if (julia_controls.visible) {
    DrawJuliaPanel(font_, width_, height_, julia_controls, fractal);
  }

  EndDrawing();
}

void FractalRender::SetLanguage(Language language) {
  language_ = language;
  SetWindowTitle(GetFractalTitle(fractal_type_, language_));
}

void FractalRender::SetFractalType(FractalType type) {
  fractal_type_ = type;
  SetWindowTitle(GetFractalTitle(fractal_type_, language_));
}

int FractalRender::GetJuliaSliderAt(float x, float y) const {
  const JuliaLayout layout = MakeJuliaLayout(width_, height_);
  const auto hits = [x, y](const Rectangle& track) {
    return x >= track.x - kJuliaSliderHitPaddingX &&
           x <= track.x + track.width + kJuliaSliderHitPaddingX &&
           y >= track.y - kJuliaSliderHitPaddingY &&
           y <= track.y + track.height + kJuliaSliderHitPaddingY;
  };

  if (hits(layout.real_track)) {
    return 0;
  }
  if (hits(layout.imag_track)) {
    return 1;
  }
  return -1;
}

bool FractalRender::IsPointInJuliaPanel(float x, float y) const {
  const JuliaLayout layout = MakeJuliaLayout(width_, height_);
  return CheckCollisionPointRec(Vector2{x, y}, layout.panel);
}

double FractalRender::GetJuliaSliderValue(int slider, float x) const {
  const JuliaLayout layout = MakeJuliaLayout(width_, height_);
  const Rectangle& track = slider == 0 ? layout.real_track : layout.imag_track;
  const double t =
      std::clamp((static_cast<double>(x) - track.x) / track.width, 0.0, 1.0);
  return kJuliaMin + t * (kJuliaMax - kJuliaMin);
}

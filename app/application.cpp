#include "application.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <raylib.h>

#include "color_map.hpp"
#include "export_presets.hpp"
#include "fractal_render.hpp"
#include "fractal_type.hpp"
#include "image_export_worker.hpp"
#include "iterations.hpp"
#include "localization.hpp"
#include "render_settings.hpp"
#include "render_worker.hpp"
#include "settings.hpp"
#include "view.hpp"
#include "view_snapshot.hpp"

namespace {

constexpr double kJuliaInitialSpan = 4.0;
constexpr double kBurningShipInitialCenterX = -0.5;
constexpr double kBurningShipInitialCenterY = 0.5;
constexpr double kBurningShipInitialSpan = 4.0;

constexpr double kIterationRepeatDelay = 0.15;

constexpr int kPreviewScale = 2;
constexpr double kFullRenderDelay = 0.2;
constexpr double kKeyboardPanSpeed = 720.0;
constexpr double kMaxFrameTime = 0.05;
constexpr double kZoomFactor = 1.2;
// Long coordinate lines do not fit in a narrow window.
constexpr int kWideWindowWidth = 900;
constexpr int kHelpMinWindowWidth = 800;
constexpr double kNoticeDuration = 3.0;
constexpr double kMillisecondsPerSecond = 1000.0;

std::string MakeScreenshotPath(const char* prefix, const char* suffix) {
  const std::time_t now = std::time(nullptr);

  std::tm local{};
#if defined(_WIN32)
  const bool has_time = localtime_s(&local, &now) == 0;
#else
  const bool has_time = localtime_r(&now, &local) != nullptr;
#endif

  std::string path = prefix;
  if (has_time) {
    char stamp[32];
    if (std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &local) > 0) {
      path += '_';
      path += stamp;
    }
  }

  path += suffix;
  path += ".png";
  return path;
}

std::string MakeInfo(const View& view, int max_iter, const char* palette,
                     double render_ms, const char* export_label,
                     const UiText& text) {
  char buffer[256];
  std::snprintf(buffer, sizeof(buffer), text.info_format,
                kInitialSpan / view.GetSpan(), max_iter, palette, render_ms,
                export_label);
  return buffer;
}

std::string MakeDetails(const View& view, const ComplexPoint& cursor,
                        bool wide_window, const UiText& text) {
  char buffer[256];
  std::snprintf(buffer, sizeof(buffer),
                wide_window ? text.details_format : text.details_narrow_format,
                view.GetCenterX(), view.GetCenterY(), cursor.real, cursor.imag);
  return buffer;
}

// Initial view: Julia at zero, Burning Ship around (-0.5, 0.5).
View MakeInitialView(FractalType type, int width, int height) {
  switch (type) {
    case FractalType::kJulia:
      return View(0.0, 0.0, kJuliaInitialSpan, width, height);
    case FractalType::kBurningShip:
      return View(kBurningShipInitialCenterX, kBurningShipInitialCenterY,
                  kBurningShipInitialSpan, width, height);
    case FractalType::kMandelbrot:
      break;
  }
  return View(width, height);
}

void ApplyWindowIcon() {
  std::string directory = GetApplicationDirectory();
  if (!directory.empty() && directory.back() != '/' &&
      directory.back() != '\\') {
    directory += '/';
  }

  const std::string path = directory + "assets/icon.png";
  Image icon = LoadImage(path.c_str());
  if (icon.data == nullptr) {
    return;
  }

  SetWindowIcon(icon);
  UnloadImage(icon);
}

ColorMap MakeColormap(const std::string& name) {
  ColorMap colormap;
  colormap.SetPalette(name);
  return colormap;
}

int GetTotalThreadCount() {
  const unsigned int hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());
  return std::max(2, static_cast<int>(hardware_threads) - 1);
}

int GetExportThreadCount() {
  return std::max(1, GetTotalThreadCount() / 4);
}

int GetDisplayThreadCount() {
  return std::max(1, GetTotalThreadCount() - GetExportThreadCount());
}

}  // namespace

Application::Application()
    : settings_file_(GetSettingsPath()),
      settings_(LoadSettings(settings_file_, GetLegacySettingsPath())),
      language_(settings_.language),
      fractal_(settings_.fractal),
      renderer_(kDefaultImageWidth, kDefaultImageHeight,
                GetFractalTitle(fractal_.type, language_)),
      colormap_(MakeColormap(settings_.palette)),
      view_(MakeInitialView(fractal_.type, renderer_.GetWidth(),
                            renderer_.GetHeight())),
      render_worker_(GetDisplayThreadCount()),
      export_worker_(GetExportThreadCount()),
      ui_(&GetUiText(language_)),
      render_state_(view_),
      export_state_(GetExportPresetIndex(settings_.export_preset)) {
  renderer_.SetLanguage(language_);
  renderer_.SetFractalType(fractal_.type);
  settings_.palette = colormap_.GetPaletteName();
  render_state_.max_iter =
      ComputeIterationsForView(view_.GetSpan(), iteration_bonus_);
  ApplyWindowIcon();
}

int Application::Run() {
  RequestFull();

  while (!WindowShouldClose()) {
    HandleResize();
    HandleMouseInput();
    HandleKeyboardInput();
    UpdateRenderRequests();
    PollWorkers();
    HandleExportKey();
    DrawFrame();
  }

  PersistSettings();
  return 0;
}

void Application::HandleResize() {
  // The window may be resized or minimized. A zero size belongs to a
  // minimized window, so no frame is requested.
  const int screen_width = GetScreenWidth();
  const int screen_height = GetScreenHeight();
  if (screen_width > 0 && screen_height > 0 &&
      (screen_width != view_.GetWidth() ||
       screen_height != view_.GetHeight())) {
    renderer_.Resize(screen_width, screen_height);
    view_.Resize(screen_width, screen_height);
    RequestPreview();
  }
}

void Application::HandleMouseInput() {
  const Vector2 mouse = GetMousePosition();
  HandleJuliaControls(mouse.x, mouse.y);
  HandlePan();
  HandleZoom(mouse.x, mouse.y);
}

void Application::HandleJuliaControls(float mouse_x, float mouse_y) {
  // The sliders are shown only for Julia.
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    ui_state_.suppress_pan = fractal_.type == FractalType::kJulia &&
                             renderer_.IsPointInJuliaPanel(mouse_x, mouse_y);
    if (fractal_.type == FractalType::kJulia) {
      const int slider = renderer_.GetJuliaSliderAt(mouse_x, mouse_y);
      if (slider >= 0) {
        ui_state_.active_julia_slider = slider;
        ApplyJuliaSlider(slider, mouse_x);
      }
    }
  }

  if (ui_state_.active_julia_slider >= 0) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      ApplyJuliaSlider(ui_state_.active_julia_slider, mouse_x);
    } else {
      // The button was released or the window lost focus.
      ui_state_.active_julia_slider = -1;
      PersistSettings();
      RequestFull();
    }
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    ui_state_.suppress_pan = false;
  }
}

void Application::HandlePan() {
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
      ui_state_.active_julia_slider < 0 && !ui_state_.suppress_pan) {
    const Vector2 delta = GetMouseDelta();
    if (delta.x != 0.0f || delta.y != 0.0f) {
      view_.PanBy(delta.x, delta.y);
      RequestPreview();
    }
  }
}

void Application::HandleZoom(float mouse_x, float mouse_y) {
  const float wheel = GetMouseWheelMove();
  if (wheel != 0.0f) {
    const double factor = std::pow(kZoomFactor, -static_cast<double>(wheel));
    if (view_.ZoomAt(factor, mouse_x, mouse_y)) {
      RequestPreview();
    }
  }
}

void Application::HandleKeyboardInput() {
  const bool shift_down =
      IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

  HandleExportPresetKeys(shift_down);
  HandleNavigationKeys(shift_down);
  HandleIterationKeys();
  HandlePaletteKey();
  HandleClipboardKeys();
  HandleViewKeys();
  HandleModeKeys();
}

void Application::HandleExportPresetKeys(bool shift_down) {
  // Shift with the arrow keys changes the PNG resolution and does not move
  // the camera.
  if (shift_down && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
    const int delta = IsKeyPressed(KEY_RIGHT) ? 1 : -1;
    export_state_.index = AdjustExportPresetIndex(export_state_.index, delta);
    settings_.export_preset = kExportPresets[export_state_.index].label;
    PersistSettings();
  }
}

void Application::HandleNavigationKeys(bool shift_down) {
  if (!shift_down) {
    const double pan_step =
        kKeyboardPanSpeed *
        std::min(static_cast<double>(GetFrameTime()), kMaxFrameTime);
    double keyboard_pan_x = 0.0;
    double keyboard_pan_y = 0.0;
    // The arrow keys move the camera in their direction, as in maps.
    if (IsKeyDown(KEY_LEFT)) {
      keyboard_pan_x += pan_step;
    }
    if (IsKeyDown(KEY_RIGHT)) {
      keyboard_pan_x -= pan_step;
    }
    if (IsKeyDown(KEY_UP)) {
      keyboard_pan_y += pan_step;
    }
    if (IsKeyDown(KEY_DOWN)) {
      keyboard_pan_y -= pan_step;
    }
    if (keyboard_pan_x != 0.0 || keyboard_pan_y != 0.0) {
      view_.PanBy(keyboard_pan_x, keyboard_pan_y);
      RequestPreview();
    }
  }

  const bool keyboard_zoom_in =
      IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD);
  const bool keyboard_zoom_out =
      IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT);
  if (keyboard_zoom_in || keyboard_zoom_out) {
    const double factor = keyboard_zoom_in ? 1.0 / kZoomFactor : kZoomFactor;
    const double center_x = view_.GetWidth() * 0.5;
    const double center_y = view_.GetHeight() * 0.5;
    if (view_.ZoomAt(factor, center_x, center_y)) {
      RequestPreview();
    }
  }
}

void Application::HandleIterationKeys() {
  int iteration_delta = 0;
  if (IsKeyDown(KEY_RIGHT_BRACKET)) {
    iteration_delta += kIterationStep;
  }
  if (IsKeyDown(KEY_LEFT_BRACKET)) {
    iteration_delta -= kIterationStep;
  }

  // The first press takes effect immediately; then the step repeats while
  // the key is held.
  const bool iteration_pressed =
      IsKeyPressed(KEY_LEFT_BRACKET) || IsKeyPressed(KEY_RIGHT_BRACKET);
  if (iteration_delta != 0 &&
      (iteration_pressed || GetTime() >= iteration_repeat_time_)) {
    int next_bonus = 0;
    if (TryNextIterationBonus(view_.GetSpan(), iteration_bonus_,
                              iteration_delta, next_bonus)) {
      iteration_bonus_ = next_bonus;
      RequestFull();
    }
    iteration_repeat_time_ = GetTime() + kIterationRepeatDelay;
  }
}

void Application::HandlePaletteKey() {
  if (!IsKeyPressed(KEY_P)) {
    return;
  }

  colormap_.SelectNextPalette();
  settings_.palette = colormap_.GetPaletteName();
  PersistSettings();
  if (!render_state_.data.empty()) {
    renderer_.Draw(render_state_.data, colormap_, render_state_.max_iter,
                   render_state_.data_width, render_state_.data_height);
  }
}

void Application::HandleClipboardKeys() {
  if (IsKeyPressed(KEY_C)) {
    HandleCopyKey();
  }
  if (IsKeyPressed(KEY_V)) {
    HandlePasteKey();
  }
}

void Application::HandleCopyKey() {
  // Use iterations for the current view; otherwise the clipboard would get
  // the value from the previous frame.
  const int clipboard_iterations =
      ComputeIterationsForView(view_.GetSpan(), iteration_bonus_);
  SetClipboardText(FormatViewSnapshot(view_, clipboard_iterations,
                                      colormap_.GetPaletteName())
                       .c_str());
  ui_state_.notice = ui_->copied;
  ui_state_.notice_time = GetTime();
}

void Application::HandlePasteKey() {
  const char* clipboard = GetClipboardText();
  ViewSnapshot snapshot;
  if (clipboard == nullptr || !ParseViewSnapshot(clipboard, snapshot)) {
    ui_state_.notice = ui_->clipboard_no_view;
    ui_state_.notice_time = GetTime();
    return;
  }

  ColorMap pasted_palette = colormap_;
  if (!pasted_palette.SetPalette(snapshot.palette)) {
    ui_state_.notice = ui_->unknown_palette;
    ui_state_.notice_time = GetTime();
    return;
  }
  if (snapshot.iterations < kMinIterations ||
      snapshot.iterations > kMaxIterations) {
    ui_state_.notice = ui_->cannot_restore_iterations;
    ui_state_.notice_time = GetTime();
    return;
  }

  const View pasted_view(snapshot.center_x, snapshot.center_y, snapshot.span,
                         view_.GetWidth(), view_.GetHeight());
  const int bonus = std::clamp(
      snapshot.iterations - ComputeIterationsForSpan(pasted_view.GetSpan()),
      kMinIterationBonus, kMaxIterationBonus);
  if (ComputeIterationsForView(pasted_view.GetSpan(), bonus) !=
      snapshot.iterations) {
    ui_state_.notice = ui_->cannot_restore_iterations;
    ui_state_.notice_time = GetTime();
    return;
  }

  view_ = pasted_view;
  iteration_bonus_ = bonus;
  colormap_ = pasted_palette;
  settings_.palette = colormap_.GetPaletteName();
  PersistSettings();
  RequestFull();
  ui_state_.notice = ui_->pasted_view;
  ui_state_.notice_time = GetTime();
}

void Application::HandleViewKeys() {
  if (IsKeyPressed(KEY_R)) {
    const View initial =
        MakeInitialView(fractal_.type, view_.GetWidth(), view_.GetHeight());
    const bool at_initial = IsSameView(view_, initial) && iteration_bonus_ == 0;
    const bool initial_requested =
        render_state_.has_request &&
        IsSameView(render_state_.last_requested_view, initial) &&
        render_state_.last_requested_bonus == 0;
    if (!at_initial || !initial_requested) {
      view_ = initial;
      iteration_bonus_ = 0;
      RequestFull();
    }
  }

  if (IsKeyPressed(KEY_H)) {
    ui_state_.show_help = !ui_state_.show_help;
  }
}

void Application::HandleModeKeys() {
  if (IsKeyPressed(KEY_T)) {
    ui_state_.active_julia_slider = -1;
    ui_state_.suppress_pan = false;
    fractal_.type = GetNextFractalType(fractal_.type);
    renderer_.SetFractalType(fractal_.type);
    PersistSettings();
    RequestFull();
  }

  if (IsKeyPressed(KEY_L)) {
    language_ = GetNextLanguage(language_);
    settings_.language = language_;
    PersistSettings();
    ui_ = &GetUiText(language_);
    renderer_.SetLanguage(language_);
    ui_state_.notice.clear();
  }
}

void Application::HandleExportKey() {
  if (IsKeyPressed(KEY_F2)) {
    RequestPresetExport();
  }
}

void Application::UpdateRenderRequests() {
  // Replace the preview with a full frame after input stops.
  if (render_state_.preview_active && !render_state_.full_requested &&
      GetTime() - render_state_.last_input_time >= kFullRenderDelay) {
    RequestFull();
  }
}

void Application::PollWorkers() {
  HandleRenderResult();
  HandleExportResult();
}

void Application::HandleRenderResult() {
  // While the worker renders a new frame, the previous one stays on screen.
  RenderResult result;
  if (!render_worker_.TryTake(result)) {
    return;
  }

  render_state_.data = std::move(result.data);
  render_state_.data_width = result.width;
  render_state_.data_height = result.height;
  render_state_.max_iter = result.max_iterations;
  render_state_.render_ms = result.render_ms;
  renderer_.Draw(render_state_.data, colormap_, render_state_.max_iter,
                 render_state_.data_width, render_state_.data_height);
  render_state_.last_frame_time = GetTime();

  // A stale frame may arrive after a view or window size change.
  if (result.id == render_state_.last_full_request_id) {
    render_state_.full_requested = false;
  }
}

void Application::HandleExportResult() {
  ImageExportResult export_result;
  if (!export_worker_.TryTake(export_result)) {
    return;
  }

  if (export_result.id == export_state_.request_id) {
    ui_state_.notice = export_result.saved
                           ? ui_->saved_prefix + export_result.path
                           : ui_->save_failed_prefix + export_result.path;
    ui_state_.notice_time = GetTime();
  }
  export_state_.request_id = 0;
}

RenderSettings Application::MakeRenderSettings(int width, int height,
                                               double span) const {
  RenderSettings settings;
  settings.width = width;
  settings.height = height;
  // Iterations follow the screen span: the export is wider than the window,
  // but central detail must not be lost.
  settings.max_iterations =
      ComputeIterationsForView(view_.GetSpan(), iteration_bonus_);
  settings.fractal = fractal_;
  settings.center_x = view_.GetCenterX();
  settings.center_y = view_.GetCenterY();
  settings.span = span;
  return settings;
}

void Application::SubmitRender(int width, int height, bool full) {
  RenderRequest request;
  request.id = render_state_.next_request_id++;
  request.settings = MakeRenderSettings(width, height, view_.GetSpan());
  render_worker_.Request(request);

  if (full) {
    render_state_.last_full_request_id = request.id;
  }

  render_state_.last_requested_view = view_;
  render_state_.last_requested_bonus = iteration_bonus_;
  render_state_.has_request = true;
}

void Application::RequestPreview() {
  SubmitRender(std::max(view_.GetWidth() / kPreviewScale, 1),
               std::max(view_.GetHeight() / kPreviewScale, 1), false);
  // The preview belongs to the new view: the old full frame is no longer
  // relevant.
  render_state_.last_full_request_id = 0;
  render_state_.preview_active = true;
  render_state_.full_requested = false;
  render_state_.last_input_time = GetTime();
}

void Application::RequestFull() {
  SubmitRender(view_.GetWidth(), view_.GetHeight(), true);
  render_state_.preview_active = false;
  render_state_.full_requested = true;
  render_state_.last_input_time = GetTime();
}

void Application::RequestPresetExport() {
  if (export_state_.request_id != 0) {
    ui_state_.notice = ui_->save_in_progress;
    ui_state_.notice_time = GetTime();
    return;
  }

  const ExportPreset& preset = kExportPresets[export_state_.index];
  const double export_span = ComputeExportSpan(
      view_.GetSpan(), view_.GetWidth(), view_.GetHeight(), preset);

  ImageExportRequest request;
  request.id = render_state_.next_request_id++;
  request.settings =
      MakeRenderSettings(preset.width, preset.height, export_span);
  request.path =
      MakeScreenshotPath(GetFractalTypeName(fractal_.type), preset.suffix);
  request.colormap = colormap_;

  export_state_.request_id = request.id;
  export_state_.label = preset.label;
  export_worker_.Request(request);

  char buffer[64];
  std::snprintf(buffer, sizeof(buffer), ui_->rendering_export_format,
                preset.label);
  ui_state_.notice = buffer;
  ui_state_.notice_time = GetTime();
}

void Application::ApplyJuliaSlider(int slider, float mouse_x) {
  const double value = renderer_.GetJuliaSliderValue(slider, mouse_x);
  double& target = slider == 0 ? fractal_.julia_real : fractal_.julia_imag;
  if (value != target) {
    target = value;
    RequestPreview();
  }
}

void Application::DrawFrame() {
  const Vector2 mouse = GetMousePosition();
  const ComplexPoint cursor = view_.GetPointAt(mouse.x, mouse.y);
  const bool busy = render_worker_.IsBusy();

  const std::string info = BuildInfo();
  const std::string details = BuildDetails(cursor, busy);
  const std::string status = BuildStatus(busy);
  const std::string visible_notice = BuildNotice();

  // In a narrow window the help panel would cover the Julia sliders.
  const bool show_help_now =
      ui_state_.show_help && (fractal_.type != FractalType::kJulia ||
                              view_.GetWidth() >= kHelpMinWindowWidth);
  const JuliaControls julia_controls{fractal_.type == FractalType::kJulia,
                                     ui_state_.active_julia_slider};
  renderer_.Display(info, details, visible_notice, status, show_help_now,
                    julia_controls, fractal_);
}

std::string Application::BuildInfo() const {
  return MakeInfo(view_, render_state_.max_iter, colormap_.GetPaletteName(),
                  render_state_.render_ms,
                  kExportPresets[export_state_.index].label, *ui_);
}

std::string Application::BuildDetails(const ComplexPoint& cursor,
                                      bool busy) const {
  const bool wide_window = view_.GetWidth() >= kWideWindowWidth;
  std::string details = MakeDetails(view_, cursor, wide_window, *ui_);
  if (render_state_.last_frame_time >= 0.0 &&
      (busy || render_state_.preview_active) && wide_window) {
    char buffer[64];
    std::snprintf(
        buffer, sizeof(buffer), ui_->image_age_format,
        (GetTime() - render_state_.last_frame_time) * kMillisecondsPerSecond);
    details += buffer;
  }
  return details;
}

std::string Application::BuildStatus(bool busy) const {
  std::string status;
  if (busy) {
    status = ui_->status_rendering;
  }
  if (export_state_.request_id != 0) {
    if (!status.empty()) {
      status += " + ";
    }
    status += "PNG " + export_state_.label;
  }
  return status;
}

std::string Application::BuildNotice() const {
  if (!ui_state_.notice.empty() &&
      GetTime() - ui_state_.notice_time < kNoticeDuration) {
    return ui_state_.notice;
  }
  return {};
}

void Application::PersistSettings() {
  settings_.fractal = fractal_;
  if (!SaveSettings(settings_, settings_file_)) {
    TraceLog(LOG_WARNING, "Could not save settings: %s",
             settings_file_.string().c_str());
  }
}

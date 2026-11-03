#ifndef FRACTALGENERATION_APP_APPLICATION_HPP_
#define FRACTALGENERATION_APP_APPLICATION_HPP_

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "color_map.hpp"
#include "fractal_render.hpp"
#include "fractal_type.hpp"
#include "image_export_worker.hpp"
#include "localization.hpp"
#include "render_settings.hpp"
#include "render_worker.hpp"
#include "settings.hpp"
#include "view.hpp"

/// Application with an interactive window.
class Application {
 public:
  /// Creates the application, loads settings, and opens the window.
  Application();
  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;

  /// Runs the main loop and returns the exit code.
  int Run();

 private:
  struct RenderState {
    explicit RenderState(const View& initial_view)
        : last_requested_view(initial_view) {}

    std::vector<double> data;
    int data_width = 0;
    int data_height = 0;
    int max_iter = 0;
    double render_ms = 0.0;
    double last_frame_time = -1.0;
    bool preview_active = false;
    bool full_requested = false;
    double last_input_time = 0.0;
    bool has_request = false;
    View last_requested_view;
    int last_requested_bonus = 0;
    std::uint64_t next_request_id = 1;
    std::uint64_t last_full_request_id = 0;
  };

  struct ExportState {
    explicit ExportState(int preset_index) : index(preset_index) {}

    int index = 0;
    std::uint64_t request_id = 0;
    std::string label;
  };

  struct UiState {
    bool show_help = true;
    int active_julia_slider = -1;
    bool suppress_pan = false;
    std::string notice;
    double notice_time = 0.0;
  };

  void HandleResize();
  void HandleMouseInput();
  void HandleJuliaControls(float mouse_x, float mouse_y);
  void HandlePan();
  void HandleZoom(float mouse_x, float mouse_y);

  void HandleKeyboardInput();
  void HandleExportPresetKeys(bool shift_down);
  void HandleNavigationKeys(bool shift_down);
  void HandleIterationKeys();
  void HandlePaletteKey();
  void HandleClipboardKeys();
  void HandleCopyKey();
  void HandlePasteKey();
  void HandleViewKeys();
  void HandleModeKeys();
  void HandleExportKey();

  void UpdateRenderRequests();
  void PollWorkers();
  void HandleRenderResult();
  void HandleExportResult();

  void SubmitRender(int width, int height, bool full);
  void RequestPreview();
  void RequestFull();
  void RequestPresetExport();
  void ApplyJuliaSlider(int slider, float mouse_x);
  RenderSettings MakeRenderSettings(int width, int height, double span) const;

  void DrawFrame();
  std::string BuildInfo() const;
  std::string BuildDetails(const ComplexPoint& cursor, bool busy) const;
  std::string BuildStatus(bool busy) const;
  std::string BuildNotice() const;
  void PersistSettings();

  std::filesystem::path settings_file_;
  Settings settings_;
  Language language_;
  FractalParams fractal_;
  FractalRender renderer_;
  ColorMap colormap_;
  View view_;
  RenderWorker render_worker_;
  ImageExportWorker export_worker_;
  const UiText* ui_ = nullptr;
  int iteration_bonus_ = 0;
  double iteration_repeat_time_ = 0.0;
  RenderState render_state_;
  ExportState export_state_;
  UiState ui_state_;
};

#endif  // FRACTALGENERATION_APP_APPLICATION_HPP_

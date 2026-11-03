#ifndef FRACTALGENERATION_RENDERER_FRACTAL_RENDER_HPP_
#define FRACTALGENERATION_RENDERER_FRACTAL_RENDER_HPP_

#include <string>
#include <vector>

#include <raylib.h>

#include "color_map.hpp"
#include "fractal_type.hpp"
#include "localization.hpp"

/// State of the Julia parameter c slider panel.
struct JuliaControls {
  bool visible = false;
  /// -1: none, 0: Re, 1: Im.
  int active_slider = -1;
};

/// raylib window, fractal texture, and screen interface.
class FractalRender {
 public:
  /// Creates a raylib window of the given size.
  FractalRender(int width, int height, const char* title);
  /// Releases raylib resources and closes the window.
  ~FractalRender();

  /// Copying is disabled: Texture2D and Image are not released automatically.
  FractalRender(const FractalRender&) = delete;
  FractalRender& operator=(const FractalRender&) = delete;

  /// Synchronizes the output size with the window; the texture is recreated
  /// on the next Draw.
  void Resize(int width, int height);

  /// Returns the current output width in pixels.
  int GetWidth() const {
    return width_;
  }

  /// Returns the current output height in pixels.
  int GetHeight() const {
    return height_;
  }

  /// Copies the iteration map into the image and updates the texture.
  /// @param data map of smoothed iterations with data_width * data_height
  /// values
  /// @param max_iter threshold from which a point is considered interior
  /// The texture is scaled to the window.
  void Draw(const std::vector<double>& data, const ColorMap& colormap,
            int max_iter, int data_width, int data_height);

  /// Draws the texture, status lines, notification, and hints.
  /// @param info zoom, iterations, palette, frame time, and PNG preset
  /// @param details view center and the point under the cursor
  /// @param notice temporary notification
  /// @param status background render state
  /// @param show_help enables the help panel
  /// @param julia_controls Julia parameter c sliders
  void Display(const std::string& info, const std::string& details,
               const std::string& notice, const std::string& status,
               bool show_help, const JuliaControls& julia_controls,
               const FractalParams& fractal) const;

  /// Index of the c slider under the point.
  /// @return 0 for Re, 1 for Im, -1 if there is no hit.
  int GetJuliaSliderAt(float x, float y) const;

  /// Returns true if the point is inside the c slider panel.
  bool IsPointInJuliaPanel(float x, float y) const;

  /// Value of the c slider at coordinate x.
  /// @param slider 0 for Re, 1 for Im
  /// @return value in the range [-2, 2].
  double GetJuliaSliderValue(int slider, float x) const;

  /// Changes the window title and help panel language.
  void SetLanguage(Language language);

  /// Changes the fractal type in the window title and HUD.
  void SetFractalType(FractalType type);

 private:
  int width_;
  int height_;
  Image image_;
  Texture2D texture_;
  Font font_{};
  bool custom_font_ = false;
  Language language_ = Language::kRussian;
  FractalType fractal_type_ = FractalType::kMandelbrot;
};

#endif  // FRACTALGENERATION_RENDERER_FRACTAL_RENDER_HPP_

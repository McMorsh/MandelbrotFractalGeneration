#ifndef FRACTALGENERATION_RENDERER_RENDER_SETTINGS_HPP_
#define FRACTALGENERATION_RENDERER_RENDER_SETTINGS_HPP_

#include "fractal_type.hpp"
#include "view.hpp"

/// Default frame width in pixels.
inline constexpr int kDefaultImageWidth = 1200;
/// Default frame height in pixels.
inline constexpr int kDefaultImageHeight = 800;

/// Parameters of a single frame: what to render and where.
struct RenderSettings {
  int width = kDefaultImageWidth;
  int height = kDefaultImageHeight;
  int max_iterations = 0;
  FractalParams fractal;
  double center_x = kInitialCenterX;
  double center_y = kInitialCenterY;
  double span = kInitialSpan;
};

#endif  // FRACTALGENERATION_RENDERER_RENDER_SETTINGS_HPP_

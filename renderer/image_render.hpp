#ifndef FRACTALGENERATION_RENDERER_IMAGE_RENDER_HPP_
#define FRACTALGENERATION_RENDERER_IMAGE_RENDER_HPP_

#include <string>
#include <vector>

#include "color_map.hpp"
#include "render_settings.hpp"

/// Maximum image side length in pixels.
inline constexpr int kMaxImageDimension = 8192;

/// Maximum supported iteration count.
inline constexpr int kMaxIterationCount = 100000;

/// Checks dimensions, iteration count, span, center, and fractal parameters.
bool IsValidRenderSettings(const RenderSettings& settings);

/// Saves a ready iteration map to a PNG without a window.
/// @param data map of width * height values
/// @return false if the dimensions do not match data or the file was not
/// written.
bool SaveIterationImage(const std::string& path,
                        const std::vector<double>& data, int width, int height,
                        int max_iterations, const ColorMap& colormap);

/// Renders a fractal and saves it to a PNG.
/// @return false if the parameters are invalid or the file was not written.
bool SaveFractalImage(const std::string& path, const RenderSettings& settings,
                      const ColorMap& colormap);

#endif  // FRACTALGENERATION_RENDERER_IMAGE_RENDER_HPP_

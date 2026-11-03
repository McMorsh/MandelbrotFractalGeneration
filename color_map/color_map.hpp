#ifndef FRACTALGENERATION_COLOR_MAP_COLOR_MAP_HPP_
#define FRACTALGENERATION_COLOR_MAP_COLOR_MAP_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include <raylib.h>

/// Chooses a pixel color from a smoothed iteration count.
class ColorMap {
 public:
  /// Creates a color map with the Ember palette.
  ColorMap();

  /// Switches to the next palette in a cycle.
  void SelectNextPalette();

  /// Selects a palette by name case-insensitively: Ember, Ocean, or Mono.
  /// @return false if the name is not known.
  bool SetPalette(const std::string& name);

  /// Name of the current palette: Ember, Ocean, or Mono.
  const char* GetPaletteName() const;

  /// Color of a pixel for a smoothed iteration count.
  /// Points with iter >= max_iter are treated as interior and painted black.
  Color GetColor(double iter, int max_iter) const;

 private:
  static Color Lerp(Color from, Color to, float t);

  std::size_t palette_index_ = 0;
  std::vector<Color> colors_;
};

#endif  // FRACTALGENERATION_COLOR_MAP_COLOR_MAP_HPP_

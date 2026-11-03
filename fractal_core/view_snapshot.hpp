#ifndef FRACTALGENERATION_FRACTAL_CORE_VIEW_SNAPSHOT_HPP_
#define FRACTALGENERATION_FRACTAL_CORE_VIEW_SNAPSHOT_HPP_

#include <string>

#include "view.hpp"

/// View parameters stored in the clipboard or a note.
struct ViewSnapshot {
  double center_x = kInitialCenterX;
  double center_y = kInitialCenterY;
  double span = kInitialSpan;
  int iterations = 0;
  std::string palette;
};

/// Formats the center, span, zoom, iterations, and palette as text.
/// The center and span are printed with max_digits10 significant digits so
/// they can be restored without loss; zoom is printed for reading.
std::string FormatViewSnapshot(const View& view, int iterations,
                               const std::string& palette);

/// Reads text created by FormatViewSnapshot.
/// Unknown lines and the zoom line are skipped; span is checked against the
/// View range.
/// @param snapshot receives the result
/// @return false if required fields are missing or values are invalid.
bool ParseViewSnapshot(const std::string& text, ViewSnapshot& snapshot);

#endif  // FRACTALGENERATION_FRACTAL_CORE_VIEW_SNAPSHOT_HPP_

#ifndef FRACTALGENERATION_RENDERER_EXPORT_PRESETS_HPP_
#define FRACTALGENERATION_RENDERER_EXPORT_PRESETS_HPP_

#include <iterator>
#include <string>

/// Resolution of an exported PNG, independent of the window size.
struct ExportPreset {
  int width;
  int height;
  /// Label in the HUD, for example "4K".
  const char* label;
  /// File name suffix, for example "_4k".
  const char* suffix;
};

inline constexpr ExportPreset kExportPresets[] = {
    {1920, 1080, "FHD", "_fhd"},
    {2560, 1440, "2K", "_2k"},
    {3840, 2160, "4K", "_4k"},
    {7680, 4320, "8K", "_8k"},
};

inline constexpr int kExportPresetCount =
    static_cast<int>(std::size(kExportPresets));

/// Returns the preset index for a label.
/// @return 0 if the label is unknown.
inline int GetExportPresetIndex(const std::string& label) {
  for (int i = 0; i < kExportPresetCount; ++i) {
    if (label == kExportPresets[i].label) {
      return i;
    }
  }
  return 0;
}

/// Moves the preset index in a cycle.
inline int AdjustExportPresetIndex(int index, int delta) {
  const int count = kExportPresetCount;
  return ((index + delta) % count + count) % count;
}

/// Chooses span so the frame height matches the window and the width reaches
/// the preset aspect ratio.
inline double ComputeExportSpan(double view_span, int view_width,
                                int view_height, const ExportPreset& preset) {
  const double visible_height =
      view_span * static_cast<double>(view_height) / view_width;
  return visible_height * static_cast<double>(preset.width) / preset.height;
}

#endif  // FRACTALGENERATION_RENDERER_EXPORT_PRESETS_HPP_

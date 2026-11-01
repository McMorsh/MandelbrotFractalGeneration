#pragma once

#include <iterator>

// Разрешение экспортируемого PNG, не зависящее от размера окна.
struct ExportPreset {
	int width;
	int height;
	const char* label;
	const char* suffix;
};

inline constexpr ExportPreset kExportPresets[] = {
	{1920, 1080, "FHD", "_fhd"},
	{2560, 1440, "2K", "_2k"},
	{3840, 2160, "4K", "_4k"},
	{7680, 4320, "8K", "_8k"},
};

inline constexpr int kExportPresetCount = static_cast<int>(std::size(kExportPresets));

// Сдвигает индекс пресета по кругу.
inline int adjustExportPresetIndex(int index, int delta) {
	const int count = kExportPresetCount;
	return ((index + delta) % count + count) % count;
}

// Подбирает span кадра так, чтобы вертикальный охват совпал с окном.
// Кадр при этом расширяется по горизонтали до пропорций пресета.
inline double exportSpanFor(double viewSpan, int viewWidth, int viewHeight, const ExportPreset& preset) {
	const double visibleHeight = viewSpan * static_cast<double>(viewHeight) / viewWidth;
	return visibleHeight * static_cast<double>(preset.width) / preset.height;
}

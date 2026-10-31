#include "ViewSnapshot.h"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace {
	std::string trim(const std::string& text) {
		const std::size_t first = text.find_first_not_of(" \t\r");
		if (first == std::string::npos)
			return {};

		const std::size_t last = text.find_last_not_of(" \t\r");
		return text.substr(first, last - first + 1);
	}

	// Поле должно быть отделено пробелом, иначе "spanning tree"
	// не считается строкой span
	bool startsWithField(const std::string& line, const char* field) {
		const std::size_t length = std::char_traits<char>::length(field);
		if (line.size() <= length || line.compare(0, length, field) != 0)
			return false;
		return line[length] == ' ' || line[length] == '\t';
	}

	bool parseDouble(const std::string& text, double& value) {
		if (text.empty())
			return false;

		errno = 0;
		char* end = nullptr;
		const double parsed = std::strtod(text.c_str(), &end);
		if (end == text.c_str() || errno == ERANGE || !std::isfinite(parsed))
			return false;

		while (*end == ' ' || *end == '\t' || *end == '\r')
			++end;
		if (*end != '\0')
			return false;

		value = parsed;
		return true;
	}

	bool parseInt(const std::string& text, int& value) {
		if (text.empty())
			return false;

		errno = 0;
		char* end = nullptr;
		const long parsed = std::strtol(text.c_str(), &end, 10);
		if (end == text.c_str() || errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX)
			return false;

		while (*end == ' ' || *end == '\t' || *end == '\r')
			++end;
		if (*end != '\0')
			return false;

		value = static_cast<int>(parsed);
		return true;
	}

	// Разбирает значение вида "(-0.6, +0i)" целиком, без хвоста
	bool parseCenterValue(const std::string& value, double& centerX, double& centerY) {
		if (value.size() < 5 || value.front() != '(' || value.back() != ')')
			return false;

		const std::size_t comma = value.find(',');
		if (comma == std::string::npos || comma == 1 || comma + 2 > value.size() - 1)
			return false;

		const std::string realText = trim(value.substr(1, comma - 1));
		std::string imagText = trim(value.substr(comma + 1, value.size() - comma - 2));
		if (imagText.empty() || (imagText.back() != 'i' && imagText.back() != 'I'))
			return false;
		imagText.pop_back();

		return parseDouble(realText, centerX) && parseDouble(trim(imagText), centerY);
	}
} // namespace

std::string formatViewSnapshot(const View& view, int iterations, const std::string& palette) {
	constexpr int kDigits = std::numeric_limits<double>::max_digits10;

	char buffer[512];
	std::snprintf(buffer, sizeof(buffer), "center (%.*g, %+.*gi)\nspan %.*g\nzoom %.6gx\niterations %d\npalette %s",
				  kDigits, view.getCenterX(), kDigits, view.getCenterY(), kDigits, view.getSpan(),
				  kInitialSpan / view.getSpan(), iterations, palette.c_str());
	return buffer;
}

bool parseViewSnapshot(const std::string& text, ViewSnapshot& snapshot) {
	ViewSnapshot parsed;
	bool hasCenter = false;
	bool hasSpan = false;
	bool hasIterations = false;
	bool hasPalette = false;

	std::size_t start = 0;
	while (start <= text.size()) {
		const std::size_t end = text.find('\n', start);
		const std::string line = trim(text.substr(start, end == std::string::npos ? std::string::npos : end - start));

		if (!line.empty()) {
			// Некорректная строка нужного вида игнорируется: обязательные поля
			// всё равно должны найтись, а посторонний текст не мешает вставке.
			if (startsWithField(line, "center")) {
				double centerX = 0.0;
				double centerY = 0.0;
				if (parseCenterValue(trim(line.substr(6)), centerX, centerY)) {
					if (hasCenter)
						return false;
					parsed.centerX = centerX;
					parsed.centerY = centerY;
					hasCenter = true;
				}
			} else if (startsWithField(line, "span")) {
				double span = 0.0;
				if (parseDouble(trim(line.substr(4)), span)) {
					if (hasSpan)
						return false;
					parsed.span = span;
					hasSpan = true;
				}
			} else if (startsWithField(line, "iterations")) {
				int iterations = 0;
				if (parseInt(trim(line.substr(10)), iterations)) {
					if (hasIterations)
						return false;
					parsed.iterations = iterations;
					hasIterations = true;
				}
			} else if (startsWithField(line, "palette")) {
				const std::string palette = trim(line.substr(7));
				if (!palette.empty()) {
					if (hasPalette)
						return false;
					parsed.palette = palette;
					hasPalette = true;
				}
			}
		}

		if (end == std::string::npos)
			break;
		start = end + 1;
	}

	if (!hasCenter || !hasSpan || !hasIterations || !hasPalette)
		return false;
	if (!(parsed.span > 0.0) || !std::isfinite(parsed.span))
		return false;
	if (parsed.span < kMinSpan || parsed.span > kMaxSpan)
		return false;
	if (parsed.iterations <= 0)
		return false;

	snapshot = parsed;
	return true;
}

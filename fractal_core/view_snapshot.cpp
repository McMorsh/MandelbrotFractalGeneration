#include "view_snapshot.hpp"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>

#include "view.hpp"

namespace {

constexpr std::size_t kCenterFieldLength = 6;       // "center"
constexpr std::size_t kSpanFieldLength = 4;         // "span"
constexpr std::size_t kIterationsFieldLength = 10;  // "iterations"
constexpr std::size_t kPaletteFieldLength = 7;      // "palette"
constexpr std::size_t kMinCenterValueLength = 5;

std::string Trim(const std::string& text) {
  const std::size_t first = text.find_first_not_of(" \t\r");
  if (first == std::string::npos) {
    return {};
  }

  const std::size_t last = text.find_last_not_of(" \t\r");
  return text.substr(first, last - first + 1);
}

// The field must be followed by a space; otherwise "spanning tree" would be
// treated as a span line.
bool StartsWithField(const std::string& line, const char* field) {
  const std::size_t length = std::char_traits<char>::length(field);
  if (line.size() <= length || line.compare(0, length, field) != 0) {
    return false;
  }
  return line[length] == ' ' || line[length] == '\t';
}

bool ParseDouble(const std::string& text, double& value) {
  if (text.empty()) {
    return false;
  }

  errno = 0;
  char* end = nullptr;
  const double parsed = std::strtod(text.c_str(), &end);
  if (end == text.c_str() || errno == ERANGE || !std::isfinite(parsed)) {
    return false;
  }

  while (*end == ' ' || *end == '\t' || *end == '\r') {
    ++end;
  }
  if (*end != '\0') {
    return false;
  }

  value = parsed;
  return true;
}

bool ParseInt(const std::string& text, int& value) {
  if (text.empty()) {
    return false;
  }

  errno = 0;
  char* end = nullptr;
  const long parsed = std::strtol(text.c_str(), &end, 10);
  if (end == text.c_str() || errno == ERANGE || parsed < INT_MIN ||
      parsed > INT_MAX) {
    return false;
  }

  while (*end == ' ' || *end == '\t' || *end == '\r') {
    ++end;
  }
  if (*end != '\0') {
    return false;
  }

  value = static_cast<int>(parsed);
  return true;
}

// Parses a value like "(-0.6, +0i)" as a whole, without trailing text.
bool ParseCenterValue(const std::string& value, double& center_x,
                      double& center_y) {
  if (value.size() < kMinCenterValueLength || value.front() != '(' ||
      value.back() != ')') {
    return false;
  }

  const std::size_t comma = value.find(',');
  if (comma == std::string::npos || comma == 1 ||
      comma + 2 > value.size() - 1) {
    return false;
  }

  const std::string real_text = Trim(value.substr(1, comma - 1));
  std::string imag_text =
      Trim(value.substr(comma + 1, value.size() - comma - 2));
  if (imag_text.empty() ||
      (imag_text.back() != 'i' && imag_text.back() != 'I')) {
    return false;
  }
  imag_text.pop_back();

  return ParseDouble(real_text, center_x) &&
         ParseDouble(Trim(imag_text), center_y);
}

struct SnapshotParseState {
  ViewSnapshot snapshot;
  bool has_center = false;
  bool has_span = false;
  bool has_iterations = false;
  bool has_palette = false;
};

bool ParseSnapshotLine(const std::string& line, SnapshotParseState& state) {
  // A malformed line of an expected kind is ignored: required fields must
  // still be found, and unrelated text should not prevent pasting.
  if (StartsWithField(line, "center")) {
    double center_x = 0.0;
    double center_y = 0.0;
    if (ParseCenterValue(Trim(line.substr(kCenterFieldLength)), center_x,
                         center_y)) {
      if (state.has_center) {
        return false;
      }
      state.snapshot.center_x = center_x;
      state.snapshot.center_y = center_y;
      state.has_center = true;
    }
  } else if (StartsWithField(line, "span")) {
    double span = 0.0;
    if (ParseDouble(Trim(line.substr(kSpanFieldLength)), span)) {
      if (state.has_span) {
        return false;
      }
      state.snapshot.span = span;
      state.has_span = true;
    }
  } else if (StartsWithField(line, "iterations")) {
    int iterations = 0;
    if (ParseInt(Trim(line.substr(kIterationsFieldLength)), iterations)) {
      if (state.has_iterations) {
        return false;
      }
      state.snapshot.iterations = iterations;
      state.has_iterations = true;
    }
  } else if (StartsWithField(line, "palette")) {
    const std::string palette = Trim(line.substr(kPaletteFieldLength));
    if (!palette.empty()) {
      if (state.has_palette) {
        return false;
      }
      state.snapshot.palette = palette;
      state.has_palette = true;
    }
  }
  return true;
}

}  // namespace

std::string FormatViewSnapshot(const View& view, int iterations,
                               const std::string& palette) {
  constexpr int kDigits = std::numeric_limits<double>::max_digits10;

  char buffer[512];
  std::snprintf(buffer, sizeof(buffer),
                "center (%.*g, %+.*gi)\nspan %.*g\nzoom %.6gx\niterations "
                "%d\npalette %s",
                kDigits, view.GetCenterX(), kDigits, view.GetCenterY(), kDigits,
                view.GetSpan(), kInitialSpan / view.GetSpan(), iterations,
                palette.c_str());
  return buffer;
}

bool ParseViewSnapshot(const std::string& text, ViewSnapshot& snapshot) {
  SnapshotParseState state;

  std::size_t start = 0;
  while (start <= text.size()) {
    const std::size_t end = text.find('\n', start);
    const std::string line = Trim(text.substr(
        start, end == std::string::npos ? std::string::npos : end - start));

    if (!line.empty() && !ParseSnapshotLine(line, state)) {
      return false;
    }

    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }

  if (!state.has_center || !state.has_span || !state.has_iterations ||
      !state.has_palette) {
    return false;
  }
  if (!(state.snapshot.span > 0.0) || !std::isfinite(state.snapshot.span)) {
    return false;
  }
  if (state.snapshot.span < kMinSpan || state.snapshot.span > kMaxSpan) {
    return false;
  }
  if (state.snapshot.iterations <= 0) {
    return false;
  }

  snapshot = state.snapshot;
  return true;
}

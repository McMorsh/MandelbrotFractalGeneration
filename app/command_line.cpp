#include "command_line.hpp"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "fractal_type.hpp"

namespace {

bool ParseInt(const char* text, int& value) {
  errno = 0;
  char* end = nullptr;
  const long parsed = std::strtol(text, &end, 10);
  if (end == text || *end != '\0' || errno == ERANGE || parsed < INT_MIN ||
      parsed > INT_MAX) {
    return false;
  }

  value = static_cast<int>(parsed);
  return true;
}

bool ParseDouble(const char* text, double& value) {
  errno = 0;
  char* end = nullptr;
  const double parsed = std::strtod(text, &end);
  if (end == text || *end != '\0' || errno == ERANGE ||
      !std::isfinite(parsed)) {
    return false;
  }

  value = parsed;
  return true;
}

}  // namespace

void PrintUsage(const char* program) {
  std::printf(
      "Usage: %s [options]\n"
      "\n"
      "Without options starts the interactive viewer.\n"
      "\n"
      "  -o, --output <file>       render to PNG without a window\n"
      "      --width <pixels>      image width (default 1200)\n"
      "      --height <pixels>     image height (default 800)\n"
      "      --iterations <count>  maximum iterations (default: auto)\n"
      "      --center <re> <im>    view center (default -0.6 0)\n"
      "      --span <value>        view width in complex units (default "
      "3.5)\n"
      "      --palette <name>      Ember, Ocean or Mono (default Ember)\n"
      "      --fractal <name>      mandelbrot, julia or burningship "
      "(default mandelbrot)\n"
      "  -h, --help                show this help\n",
      program);
}

bool ParseArguments(int argc, char** argv, Options& options) {
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      options.show_help = true;
      return true;
    }

    if (arg == "--output" || arg == "-o") {
      if (i + 1 >= argc) {
        return false;
      }
      options.output = argv[++i];
      options.headless = true;
      continue;
    }

    if (arg == "--width") {
      if (i + 1 >= argc || !ParseInt(argv[i + 1], options.settings.width)) {
        return false;
      }
      ++i;
      continue;
    }

    if (arg == "--height") {
      if (i + 1 >= argc || !ParseInt(argv[i + 1], options.settings.height)) {
        return false;
      }
      ++i;
      continue;
    }

    if (arg == "--iterations") {
      if (i + 1 >= argc ||
          !ParseInt(argv[i + 1], options.settings.max_iterations)) {
        return false;
      }
      ++i;
      continue;
    }

    if (arg == "--center") {
      if (i + 2 >= argc) {
        return false;
      }
      if (!ParseDouble(argv[i + 1], options.settings.center_x)) {
        return false;
      }
      if (!ParseDouble(argv[i + 2], options.settings.center_y)) {
        return false;
      }
      i += 2;
      continue;
    }

    if (arg == "--span") {
      if (i + 1 >= argc || !ParseDouble(argv[i + 1], options.settings.span)) {
        return false;
      }
      ++i;
      continue;
    }

    if (arg == "--palette") {
      if (i + 1 >= argc) {
        return false;
      }
      options.palette = argv[++i];
      continue;
    }

    if (arg == "--fractal") {
      if (i + 1 >= argc ||
          !ParseFractalType(argv[i + 1], options.settings.fractal.type)) {
        return false;
      }
      ++i;
      continue;
    }

    return false;
  }

  return true;
}

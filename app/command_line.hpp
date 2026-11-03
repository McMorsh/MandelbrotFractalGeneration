#ifndef FRACTALGENERATION_APP_COMMAND_LINE_HPP_
#define FRACTALGENERATION_APP_COMMAND_LINE_HPP_

#include <string>

#include "render_settings.hpp"

/// Command-line options.
struct Options {
  bool show_help = false;
  bool headless = false;
  std::string output;
  std::string palette = "Ember";
  RenderSettings settings;
};

/// Parses command-line arguments.
/// @return false if an argument is unknown or its value is missing.
bool ParseArguments(int argc, char** argv, Options& options);

/// Prints command-line help.
void PrintUsage(const char* program);

#endif  // FRACTALGENERATION_APP_COMMAND_LINE_HPP_

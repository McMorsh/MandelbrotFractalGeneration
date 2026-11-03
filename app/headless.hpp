#ifndef FRACTALGENERATION_APP_HEADLESS_HPP_
#define FRACTALGENERATION_APP_HEADLESS_HPP_

#include "command_line.hpp"

/// Renders a PNG without a window.
/// @return process exit code: 0 on success, 1 on error.
int RunHeadless(const Options& options);

#endif  // FRACTALGENERATION_APP_HEADLESS_HPP_

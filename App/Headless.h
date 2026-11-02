#pragma once

#include "CommandLine.h"

/// Рендерит PNG без окна.
/// @return код возврата процесса: 0 при успехе, 1 при ошибке.
int runHeadless(const Options& options);

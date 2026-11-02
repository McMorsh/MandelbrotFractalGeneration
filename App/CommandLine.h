#pragma once

#include "RenderSettings.h"

#include <string>

/// Параметры командной строки.
struct Options {
	bool showHelp = false;
	bool headless = false;
	std::string output;
	std::string palette = "Ember";
	RenderSettings settings;
};

/// Разбирает аргументы командной строки.
/// @return false, если аргумент неизвестен или для него не хватает значения.
bool parseArguments(int argc, char** argv, Options& options);

/// Печатает справку по аргументам.
void printUsage(const char* program);

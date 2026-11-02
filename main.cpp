#include "Application.h"
#include "CommandLine.h"
#include "Headless.h"

#include <cstdio>

int main(int argc, char** argv) {
	Options options;
	if (!parseArguments(argc, argv, options)) {
		std::fprintf(stderr, "Invalid arguments.\n");
		printUsage(argv[0]);
		return 1;
	}

	if (options.showHelp) {
		printUsage(argv[0]);
		return 0;
	}

	if (options.headless)
		return runHeadless(options);

	Application app;
	return app.run();
}

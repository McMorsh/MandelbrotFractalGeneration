#include <cstdio>

#include "application.hpp"
#include "command_line.hpp"
#include "headless.hpp"

int main(int argc, char** argv) {
  Options options;
  if (!ParseArguments(argc, argv, options)) {
    std::fprintf(stderr, "Invalid arguments.\n");
    PrintUsage(argv[0]);
    return 1;
  }

  if (options.show_help) {
    PrintUsage(argv[0]);
    return 0;
  }

  if (options.headless) {
    return RunHeadless(options);
  }

  Application app;
  return app.Run();
}

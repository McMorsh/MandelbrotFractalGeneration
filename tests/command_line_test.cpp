#include "CommandLine.h"

#include "FractalType.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace {
	struct Args {
		std::vector<std::string> values;
		std::vector<char*> pointers;

		explicit Args(std::initializer_list<const char*> list) {
			values.reserve(list.size());
			for (const char* value : list)
				values.emplace_back(value);
			pointers.reserve(values.size());
			for (auto& value : values)
				pointers.push_back(value.data());
		}

		int argc() const {
			return static_cast<int>(pointers.size());
		}

		char** argv() {
			return pointers.data();
		}
	};

	TEST(CommandLineTest, ParsesHelp) {
		Args args{"FractalGeneration", "--help"};
		Options options;

		ASSERT_TRUE(parseArguments(args.argc(), args.argv(), options));
		EXPECT_TRUE(options.showHelp);
	}

	TEST(CommandLineTest, ParsesHeadlessOutput) {
		Args args{"FractalGeneration", "-o", "out.png"};
		Options options;

		ASSERT_TRUE(parseArguments(args.argc(), args.argv(), options));
		EXPECT_TRUE(options.headless);
		EXPECT_EQ(options.output, "out.png");
	}

	TEST(CommandLineTest, ParsesFractalAndPalette) {
		Args args{"FractalGeneration", "--fractal", "burningship", "--palette", "Ocean"};
		Options options;

		ASSERT_TRUE(parseArguments(args.argc(), args.argv(), options));
		EXPECT_EQ(options.settings.fractal.type, FractalType::BurningShip);
		EXPECT_EQ(options.palette, "Ocean");
	}

	TEST(CommandLineTest, ParsesRenderSettings) {
		Args args{"FractalGeneration", "--width", "640", "--height", "480", "--iterations", "200",
				  "--center",		   "-0.5",	  "0.5", "--span",	 "2"};
		Options options;

		ASSERT_TRUE(parseArguments(args.argc(), args.argv(), options));
		EXPECT_EQ(options.settings.width, 640);
		EXPECT_EQ(options.settings.height, 480);
		EXPECT_EQ(options.settings.maxIterations, 200);
		EXPECT_DOUBLE_EQ(options.settings.centerX, -0.5);
		EXPECT_DOUBLE_EQ(options.settings.centerY, 0.5);
		EXPECT_DOUBLE_EQ(options.settings.span, 2.0);
	}

	TEST(CommandLineTest, RejectsUnknownOption) {
		Args args{"FractalGeneration", "--unknown"};
		Options options;

		EXPECT_FALSE(parseArguments(args.argc(), args.argv(), options));
	}

	TEST(CommandLineTest, RejectsMissingValue) {
		Args args{"FractalGeneration", "--width"};
		Options options;

		EXPECT_FALSE(parseArguments(args.argc(), args.argv(), options));
	}
} // namespace

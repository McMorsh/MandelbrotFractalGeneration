#include "ColorMap.h"
#include "FractalRender.h"
#include "ImageExportWorker.h"
#include "ImageRender.h"
#include "Iterations.h"
#include "RenderWorker.h"
#include "View.h"
#include "ViewSnapshot.h"

#include <raylib.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {
	constexpr int kWindowWidth = 1200;
	constexpr int kWindowHeight = 800;

	constexpr double kIterationRepeatDelay = 0.15;

	constexpr int kPreviewScale = 2;
	constexpr int kSaveScale = 2;
	constexpr double kFullRenderDelay = 0.2;
	constexpr double kKeyboardPanSpeed = 720.0;
	constexpr double kMaxFrameTime = 0.05;
	constexpr double kZoomFactor = 1.2;
	// На узком окне длинные строки с координатами не помещаются
	constexpr int kWideWindowWidth = 900;

	std::string screenshotPath(const char* suffix = "") {
		const std::time_t now = std::time(nullptr);

		std::tm local{};
#if defined(_WIN32)
		const bool hasTime = localtime_s(&local, &now) == 0;
#else
		const bool hasTime = localtime_r(&now, &local) != nullptr;
#endif

		std::string path = "mandelbrot";
		if (hasTime) {
			char buffer[64];
			if (std::strftime(buffer, sizeof(buffer), "mandelbrot_%Y%m%d_%H%M%S", &local) > 0)
				path = buffer;
		}

		path += suffix;
		path += ".png";
		return path;
	}

	std::string makeInfo(const View& view, int maxIter, const char* palette, double renderMs) {
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "zoom %.3gx   |   iterations %d   |   %s   |   render %.1f ms",
					  kInitialSpan / view.getSpan(), maxIter, palette, renderMs);
		return buffer;
	}

	std::string makeDetails(const View& view, const ComplexPoint& cursor, bool wideWindow) {
		char buffer[256];
		if (wideWindow) {
			std::snprintf(buffer, sizeof(buffer), "center (%.15g, %+.15gi)   |   cursor (%.15g, %+.15gi)",
						  view.getCenterX(), view.getCenterY(), cursor.real, cursor.imag);
		} else {
			// В узком окне места хватает только на курсор, но точность не теряем
			std::snprintf(buffer, sizeof(buffer), "cursor (%.15g, %+.15gi)", cursor.real, cursor.imag);
		}
		return buffer;
	}

	struct Options {
		bool showHelp = false;
		bool headless = false;
		std::string output;
		std::string palette = "Ember";
		RenderSettings settings;
	};

	bool parseInt(const char* text, int& value) {
		errno = 0;
		char* end = nullptr;
		const long parsed = std::strtol(text, &end, 10);
		if (end == text || *end != '\0' || errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX) {
			return false;
		}

		value = static_cast<int>(parsed);
		return true;
	}

	bool parseDouble(const char* text, double& value) {
		errno = 0;
		char* end = nullptr;
		const double parsed = std::strtod(text, &end);
		if (end == text || *end != '\0' || errno == ERANGE || !std::isfinite(parsed)) {
			return false;
		}

		value = parsed;
		return true;
	}

	void printUsage(const char* program) {
		std::printf("Usage: %s [options]\n"
					"\n"
					"Without options starts the interactive viewer.\n"
					"\n"
					"  -o, --output <file>       render to PNG without a window\n"
					"      --width <pixels>      image width (default 1200)\n"
					"      --height <pixels>     image height (default 800)\n"
					"      --iterations <count>  maximum iterations (default: auto)\n"
					"      --center <re> <im>    view center (default -0.6 0)\n"
					"      --span <value>        view width in complex units (default 3.5)\n"
					"      --palette <name>      Ember, Ocean or Mono (default Ember)\n"
					"  -h, --help                show this help\n",
					program);
	}

	bool parseArguments(int argc, char** argv, Options& options) {
		for (int i = 1; i < argc; ++i) {
			const std::string arg = argv[i];

			if (arg == "--help" || arg == "-h") {
				options.showHelp = true;
				return true;
			}

			if (arg == "--output" || arg == "-o") {
				if (i + 1 >= argc)
					return false;
				options.output = argv[++i];
				options.headless = true;
				continue;
			}

			if (arg == "--width") {
				if (i + 1 >= argc || !parseInt(argv[i + 1], options.settings.width))
					return false;
				++i;
				continue;
			}

			if (arg == "--height") {
				if (i + 1 >= argc || !parseInt(argv[i + 1], options.settings.height))
					return false;
				++i;
				continue;
			}

			if (arg == "--iterations") {
				if (i + 1 >= argc || !parseInt(argv[i + 1], options.settings.maxIterations))
					return false;
				++i;
				continue;
			}

			if (arg == "--center") {
				if (i + 2 >= argc)
					return false;
				if (!parseDouble(argv[i + 1], options.settings.centerX))
					return false;
				if (!parseDouble(argv[i + 2], options.settings.centerY))
					return false;
				i += 2;
				continue;
			}

			if (arg == "--span") {
				if (i + 1 >= argc || !parseDouble(argv[i + 1], options.settings.span))
					return false;
				++i;
				continue;
			}

			if (arg == "--palette") {
				if (i + 1 >= argc)
					return false;
				options.palette = argv[++i];
				continue;
			}

			return false;
		}

		return true;
	}

	int runGui() {
		FractalRender renderer(kWindowWidth, kWindowHeight, "Mandelbrot Fractal Generation");
		ColorMap colormap;
		View view(renderer.getWidth(), renderer.getHeight());
		const unsigned int hardwareThreads = std::max(1u, std::thread::hardware_concurrency());
		const int totalThreads = std::max(2, static_cast<int>(hardwareThreads) - 1);
		const int exportThreads = std::max(1, totalThreads / 4);
		const int displayThreads = std::max(1, totalThreads - exportThreads);

		RenderWorker worker(displayThreads);
		ImageExportWorker exportWorker(exportThreads);

		std::uint64_t nextRequestId = 1;
		std::uint64_t lastFullRequestId = 0;
		std::uint64_t displayedRequestId = 0;

		bool showHelp = true;
		int iterationBonus = 0;
		int maxIter = iterationsForView(view.getSpan(), iterationBonus);
		double renderMs = 0.0;
		double lastFrameTime = -1.0;

		std::vector<double> data;
		int dataWidth = 0;
		int dataHeight = 0;

		bool previewActive = false;
		bool fullRequested = false;
		double lastInputTime = 0.0;
		double iterationRepeatTime = 0.0;
		bool hasRequest = false;
		View lastRequestedView(view);
		int lastRequestedBonus = 0;

		bool saveRequested = false;
		std::uint64_t saveRequestId = 0;
		std::string notice;
		double noticeTime = 0.0;

		auto submitRender = [&](int renderWidth, int renderHeight, bool full) {
			RenderRequest request;
			request.id = nextRequestId++;
			request.width = renderWidth;
			request.height = renderHeight;
			request.maxIterations = iterationsForView(view.getSpan(), iterationBonus);
			request.centerX = view.getCenterX();
			request.centerY = view.getCenterY();
			request.span = view.getSpan();
			worker.request(request);

			if (full) {
				lastFullRequestId = request.id;
			}

			lastRequestedView = view;
			lastRequestedBonus = iterationBonus;
			hasRequest = true;
		};

		auto requestPreview = [&]() {
			submitRender(std::max(view.getWidth() / kPreviewScale, 1), std::max(view.getHeight() / kPreviewScale, 1),
						 false);
			// Предпросмотр относится к новому виду: старый полный кадр больше не актуален
			lastFullRequestId = 0;
			previewActive = true;
			fullRequested = false;
			lastInputTime = GetTime();
		};

		auto requestFull = [&]() {
			submitRender(view.getWidth(), view.getHeight(), true);
			previewActive = false;
			fullRequested = true;
			lastInputTime = GetTime();
		};

		auto saveCurrentFrame = [&]() {
			const std::string path = screenshotPath();
			notice = renderer.saveImage(path) ? "Saved " + path : "Could not save " + path;
			noticeTime = GetTime();
		};

		auto requestHighResolutionSave = [&]() {
			if (saveRequestId != 0) {
				notice = "Save already in progress";
				noticeTime = GetTime();
				return;
			}

			const int saveWidth = view.getWidth() * kSaveScale;
			const int saveHeight = view.getHeight() * kSaveScale;
			if (saveWidth > kMaxImageDimension || saveHeight > kMaxImageDimension) {
				notice = "2x image is too large";
				noticeTime = GetTime();
				return;
			}

			ImageExportRequest request;
			request.id = nextRequestId++;
			request.width = saveWidth;
			request.height = saveHeight;
			request.maxIterations = iterationsForView(view.getSpan(), iterationBonus);
			request.centerX = view.getCenterX();
			request.centerY = view.getCenterY();
			request.span = view.getSpan();
			request.path = screenshotPath("_2x");
			request.colormap = colormap;

			saveRequestId = request.id;
			exportWorker.request(request);

			notice = "Rendering 2x...";
			noticeTime = GetTime();
		};

		requestFull();

		while (!WindowShouldClose()) {
			bool viewChanged = false;

			// Окно могли растянуть или свернуть. Нулевой размер бывает
			// у свернутого окна, такой кадр не запрашиваем.
			const int screenWidth = GetScreenWidth();
			const int screenHeight = GetScreenHeight();
			if (screenWidth > 0 && screenHeight > 0 &&
				(screenWidth != view.getWidth() || screenHeight != view.getHeight())) {
				renderer.resize(screenWidth, screenHeight);
				view.resize(screenWidth, screenHeight);
				requestPreview();
				viewChanged = true;
			}

			const Vector2 mouse = GetMousePosition();

			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
				const Vector2 delta = GetMouseDelta();
				if (delta.x != 0.0f || delta.y != 0.0f) {
					view.panBy(delta.x, delta.y);
					requestPreview();
					viewChanged = true;
				}
			}

			const float wheel = GetMouseWheelMove();
			if (wheel != 0.0f) {
				const double factor = std::pow(kZoomFactor, -static_cast<double>(wheel));
				if (view.zoomAt(factor, mouse.x, mouse.y)) {
					requestPreview();
					viewChanged = true;
				}
			}

			const double panStep = kKeyboardPanSpeed * std::min(static_cast<double>(GetFrameTime()), kMaxFrameTime);
			double keyboardPanX = 0.0;
			double keyboardPanY = 0.0;
			// Стрелки двигают камеру в свою сторону, как в картах
			if (IsKeyDown(KEY_LEFT))
				keyboardPanX += panStep;
			if (IsKeyDown(KEY_RIGHT))
				keyboardPanX -= panStep;
			if (IsKeyDown(KEY_UP))
				keyboardPanY += panStep;
			if (IsKeyDown(KEY_DOWN))
				keyboardPanY -= panStep;
			if (keyboardPanX != 0.0 || keyboardPanY != 0.0) {
				view.panBy(keyboardPanX, keyboardPanY);
				requestPreview();
				viewChanged = true;
			}

			const bool keyboardZoomIn = IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD);
			const bool keyboardZoomOut = IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT);
			if (keyboardZoomIn || keyboardZoomOut) {
				const double factor = keyboardZoomIn ? 1.0 / kZoomFactor : kZoomFactor;
				const double centerX = view.getWidth() * 0.5;
				const double centerY = view.getHeight() * 0.5;
				if (view.zoomAt(factor, centerX, centerY)) {
					requestPreview();
					viewChanged = true;
				}
			}

			int iterationDelta = 0;
			if (IsKeyDown(KEY_RIGHT_BRACKET))
				iterationDelta += kIterationStep;
			if (IsKeyDown(KEY_LEFT_BRACKET))
				iterationDelta -= kIterationStep;

			// Первое нажатие срабатывает сразу, дальше шаг повторяется,
			// пока клавиша удерживается
			const bool iterationPressed = IsKeyPressed(KEY_LEFT_BRACKET) || IsKeyPressed(KEY_RIGHT_BRACKET);
			if (iterationDelta != 0 && (iterationPressed || GetTime() >= iterationRepeatTime)) {
				int nextBonus = 0;
				if (nextIterationBonus(view.getSpan(), iterationBonus, iterationDelta, nextBonus)) {
					iterationBonus = nextBonus;
					requestFull();
					viewChanged = true;
				}
				iterationRepeatTime = GetTime() + kIterationRepeatDelay;
			}

			if (IsKeyPressed(KEY_P)) {
				colormap.nextPalette();
				if (!data.empty()) {
					renderer.draw(data, colormap, maxIter, dataWidth, dataHeight);
				}
			}

			if (IsKeyPressed(KEY_C)) {
				// Итерации считаем по текущему виду, иначе в буфер попадёт
				// число от прошлого кадра
				const int clipboardIterations = iterationsForView(view.getSpan(), iterationBonus);
				SetClipboardText(formatViewSnapshot(view, clipboardIterations, colormap.paletteName()).c_str());
				notice = "Copied to clipboard";
				noticeTime = GetTime();
			}

			if (IsKeyPressed(KEY_V)) {
				const char* clipboard = GetClipboardText();
				ViewSnapshot snapshot;
				if (clipboard == nullptr || !parseViewSnapshot(clipboard, snapshot)) {
					notice = "Clipboard has no view data";
					noticeTime = GetTime();
				} else {
					ColorMap pastedPalette = colormap;
					if (!pastedPalette.setPalette(snapshot.palette)) {
						notice = "Unknown palette in clipboard";
						noticeTime = GetTime();
					} else if (snapshot.iterations < kMinIterations || snapshot.iterations > kMaxIterations) {
						notice = "Cannot restore iterations";
						noticeTime = GetTime();
					} else {
						const View pastedView(snapshot.centerX, snapshot.centerY, snapshot.span, view.getWidth(),
											  view.getHeight());
						const int bonus = std::clamp(snapshot.iterations - iterationsForSpan(pastedView.getSpan()),
													 kMinIterationBonus, kMaxIterationBonus);
						if (iterationsForView(pastedView.getSpan(), bonus) != snapshot.iterations) {
							notice = "Cannot restore iterations";
							noticeTime = GetTime();
						} else {
							view = pastedView;
							iterationBonus = bonus;
							colormap = pastedPalette;
							requestFull();
							viewChanged = true;
							notice = "Pasted view";
							noticeTime = GetTime();
						}
					}
				}
			}

			if (IsKeyPressed(KEY_R)) {
				const View home(view.getWidth(), view.getHeight());
				const bool atHome = sameView(view, home) && iterationBonus == 0;
				const bool homeRequested = hasRequest && sameView(lastRequestedView, home) && lastRequestedBonus == 0;
				if (!atHome || !homeRequested) {
					view = home;
					iterationBonus = 0;
					requestFull();
					viewChanged = true;
				}
			}

			if (viewChanged) {
				if (saveRequested) {
					saveRequested = false;
					notice = "Save canceled: view changed";
					noticeTime = GetTime();
				}
			}

			if (IsKeyPressed(KEY_H)) {
				showHelp = !showHelp;
			}

			// После остановки ввода заменяем предпросмотр полным кадром
			if (previewActive && !fullRequested && GetTime() - lastInputTime >= kFullRenderDelay) {
				requestFull();
			}

			// Пока воркер считает новый кадр, на экране остается предыдущий
			RenderResult result;
			if (worker.tryTake(result)) {
				data = std::move(result.data);
				dataWidth = result.width;
				dataHeight = result.height;
				maxIter = result.maxIterations;
				renderMs = result.renderMs;
				displayedRequestId = result.id;
				renderer.draw(data, colormap, maxIter, dataWidth, dataHeight);
				lastFrameTime = GetTime();

				// Устаревший кадр мог прийти после смены вида или размера окна
				if (result.id == lastFullRequestId) {
					fullRequested = false;
					if (saveRequested) {
						saveCurrentFrame();
						saveRequested = false;
					}
				}
			}

			ImageExportResult exportResult;
			if (exportWorker.tryTake(exportResult)) {
				if (exportResult.id == saveRequestId) {
					notice = exportResult.saved ? "Saved " + exportResult.path : "Could not save " + exportResult.path;
					noticeTime = GetTime();
				}
				saveRequestId = 0;
			}

			if (IsKeyPressed(KEY_F2)) {
				const bool highResolution = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
				if (highResolution) {
					requestHighResolutionSave();
				} else {
					const bool currentFullFrame =
						!previewActive && !fullRequested && displayedRequestId == lastFullRequestId;

					if (data.empty()) {
						notice = "Nothing rendered yet";
						noticeTime = GetTime();
					} else if (currentFullFrame) {
						saveCurrentFrame();
					} else {
						saveRequested = true;
						notice = "Waiting for full render";
						noticeTime = GetTime();
						if (!previewActive && !fullRequested) {
							requestFull();
						}
					}
				}
			}

			const ComplexPoint cursor = view.pointAt(mouse.x, mouse.y);
			std::string info = makeInfo(view, maxIter, colormap.paletteName(), renderMs);

			const bool busy = worker.isBusy();
			std::string status;
			if (busy)
				status = "rendering...";
			if (saveRequestId != 0) {
				if (!status.empty())
					status += "   |   ";
				status += "saving 2x...";
			}

			const bool wideWindow = view.getWidth() >= kWideWindowWidth;
			std::string details = makeDetails(view, cursor, wideWindow);
			if (lastFrameTime >= 0.0 && (busy || previewActive) && wideWindow) {
				char buffer[64];
				std::snprintf(buffer, sizeof(buffer), "   |   image age %.0f ms", (GetTime() - lastFrameTime) * 1000.0);
				details += buffer;
			}

			std::string visibleNotice;
			if (!notice.empty() && GetTime() - noticeTime < 3.0) {
				visibleNotice = notice;
			}
			renderer.display(info, details, visibleNotice, status, showHelp);
		}

		return 0;
	}
} // namespace

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

	if (!options.headless) {
		return runGui();
	}

	if (!(options.settings.span > 0.0)) {
		std::fprintf(stderr, "Span must be positive.\n");
		return 1;
	}

	if (options.settings.maxIterations == 0) {
		options.settings.maxIterations = iterationsForSpan(options.settings.span);
	}

	if (!isValidRenderSettings(options.settings)) {
		std::fprintf(stderr, "Invalid render settings. Check size, iterations and span.\n");
		return 1;
	}

	ColorMap colormap;
	if (!colormap.setPalette(options.palette)) {
		std::fprintf(stderr, "Unknown palette: %s (available: Ember, Ocean, Mono)\n", options.palette.c_str());
		return 1;
	}

	std::printf("Rendering %dx%d, %d iterations, palette %s...\n", options.settings.width, options.settings.height,
				options.settings.maxIterations, colormap.paletteName());
	std::fflush(stdout);

	if (!saveFractalImage(options.output, options.settings, colormap)) {
		std::fprintf(stderr, "Could not save %s\n", options.output.c_str());
		return 1;
	}

	std::printf("Saved %s\n", options.output.c_str());
	return 0;
}

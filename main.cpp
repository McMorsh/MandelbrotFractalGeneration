#include "ColorMap.h"
#include "ExportPresets.h"
#include "FractalRender.h"
#include "FractalType.h"
#include "ImageExportWorker.h"
#include "ImageRender.h"
#include "Iterations.h"
#include "Localization.h"
#include "RenderWorker.h"
#include "Settings.h"
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
#include <filesystem>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {
	constexpr int kWindowWidth = 1200;
	constexpr int kWindowHeight = 800;
	constexpr double kJuliaHomeSpan = 4.0;

	constexpr double kIterationRepeatDelay = 0.15;

	constexpr int kPreviewScale = 2;
	constexpr double kFullRenderDelay = 0.2;
	constexpr double kKeyboardPanSpeed = 720.0;
	constexpr double kMaxFrameTime = 0.05;
	constexpr double kZoomFactor = 1.2;
	// На узком окне длинные строки с координатами не помещаются
	constexpr int kWideWindowWidth = 900;

	std::string screenshotPath(const char* suffix) {
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

	std::string makeInfo(const View& view, int maxIter, const char* palette, double renderMs, const char* exportLabel,
						 const UiText& text) {
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), text.infoFormat, kInitialSpan / view.getSpan(), maxIter, palette,
					  renderMs, exportLabel);
		return buffer;
	}

	std::string makeDetails(const View& view, const ComplexPoint& cursor, bool wideWindow, const UiText& text) {
		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), wideWindow ? text.detailsFormat : text.detailsNarrowFormat,
					  view.getCenterX(), view.getCenterY(), cursor.real, cursor.imag);
		return buffer;
	}

	// Домашний вид зависит от фрактала: Julia симметричен относительно нуля
	View homeView(FractalType type, int width, int height) {
		if (type == FractalType::Julia)
			return View(0.0, 0.0, kJuliaHomeSpan, width, height);
		return View(width, height);
	}

	void setWindowIcon() {
		std::string directory = GetApplicationDirectory();
		if (!directory.empty() && directory.back() != '/' && directory.back() != '\\')
			directory += '/';

		const std::string path = directory + "assets/icon.png";
		Image icon = LoadImage(path.c_str());
		if (icon.data == nullptr)
			return;

		SetWindowIcon(icon);
		UnloadImage(icon);
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
					"      --fractal <name>      mandelbrot or julia (default mandelbrot)\n"
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

			if (arg == "--fractal") {
				if (i + 1 >= argc || !parseFractalType(argv[i + 1], options.settings.fractal.type))
					return false;
				++i;
				continue;
			}

			return false;
		}

		return true;
	}

	int runGui() {
		const std::filesystem::path settingsFile = settingsPath();
		Settings settings = loadSettings(settingsFile);
		Language language = settings.language;
		FractalParams fractal = settings.fractal;
		FractalRender renderer(kWindowWidth, kWindowHeight, fractalTitle(fractal.type, language));
		renderer.setLanguage(language);
		renderer.setFractalType(fractal.type);
		const UiText* ui = &uiText(language);
		ColorMap colormap;
		colormap.setPalette(settings.palette);
		settings.palette = colormap.paletteName();
		setWindowIcon();
		View view = homeView(fractal.type, renderer.getWidth(), renderer.getHeight());
		const unsigned int hardwareThreads = std::max(1u, std::thread::hardware_concurrency());
		const int totalThreads = std::max(2, static_cast<int>(hardwareThreads) - 1);
		const int exportThreads = std::max(1, totalThreads / 4);
		const int displayThreads = std::max(1, totalThreads - exportThreads);

		RenderWorker worker(displayThreads);
		ImageExportWorker exportWorker(exportThreads);

		std::uint64_t nextRequestId = 1;
		std::uint64_t lastFullRequestId = 0;

		bool showHelp = true;
		int activeJuliaSlider = -1;
		bool suppressPan = false;
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

		std::uint64_t saveRequestId = 0;
		std::string saveLabel;
		int exportIndex = exportPresetIndex(settings.exportPreset);
		std::string notice;
		double noticeTime = 0.0;

		auto submitRender = [&](int renderWidth, int renderHeight, bool full) {
			RenderRequest request;
			request.id = nextRequestId++;
			request.fractal = fractal;
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

		auto applyJuliaSlider = [&](int slider, float x) {
			const double value = renderer.juliaSliderValue(slider, x);
			double& target = slider == 0 ? fractal.juliaReal : fractal.juliaImag;
			if (value != target) {
				target = value;
				requestPreview();
			}
		};

		auto requestPresetExport = [&]() {
			if (saveRequestId != 0) {
				notice = ui->saveInProgress;
				noticeTime = GetTime();
				return;
			}

			const ExportPreset& preset = kExportPresets[exportIndex];
			const double exportSpan = exportSpanFor(view.getSpan(), view.getWidth(), view.getHeight(), preset);

			ImageExportRequest request;
			request.id = nextRequestId++;
			request.fractal = fractal;
			request.width = preset.width;
			request.height = preset.height;
			// Итерации считаем по экранному span: экспорт шире окна, и от exportSpan
			// их было бы меньше, чем на экране, из-за чего терялись бы тонкие детали
			request.maxIterations = iterationsForView(view.getSpan(), iterationBonus);
			request.centerX = view.getCenterX();
			request.centerY = view.getCenterY();
			request.span = exportSpan;
			request.path = screenshotPath(preset.suffix);
			request.colormap = colormap;

			saveRequestId = request.id;
			saveLabel = preset.label;
			exportWorker.request(request);

			char buffer[64];
			std::snprintf(buffer, sizeof(buffer), ui->renderingExportFormat, preset.label);
			notice = buffer;
			noticeTime = GetTime();
		};

		requestFull();

		while (!WindowShouldClose()) {
			// Окно могли растянуть или свернуть. Нулевой размер бывает
			// у свернутого окна, такой кадр не запрашиваем.
			const int screenWidth = GetScreenWidth();
			const int screenHeight = GetScreenHeight();
			if (screenWidth > 0 && screenHeight > 0 &&
				(screenWidth != view.getWidth() || screenHeight != view.getHeight())) {
				renderer.resize(screenWidth, screenHeight);
				view.resize(screenWidth, screenHeight);
				requestPreview();
			}

			const Vector2 mouse = GetMousePosition();

			// Ползунки c показываются только у Julia
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				suppressPan = fractal.type == FractalType::Julia && renderer.juliaPanelAt(mouse.x, mouse.y);
				if (fractal.type == FractalType::Julia) {
					const int slider = renderer.juliaSliderAt(mouse.x, mouse.y);
					if (slider >= 0) {
						activeJuliaSlider = slider;
						applyJuliaSlider(slider, mouse.x);
					}
				}
			}

			if (activeJuliaSlider >= 0) {
				if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
					applyJuliaSlider(activeJuliaSlider, mouse.x);
				} else {
					// Кнопку отпустили или окно потеряло фокус
					activeJuliaSlider = -1;
					settings.fractal = fractal;
					if (!saveSettings(settings, settingsFile))
						TraceLog(LOG_WARNING, "Could not save settings: %s", settingsFile.string().c_str());
					requestFull();
				}
			}

			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
				suppressPan = false;

			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && activeJuliaSlider < 0 && !suppressPan) {
				const Vector2 delta = GetMouseDelta();
				if (delta.x != 0.0f || delta.y != 0.0f) {
					view.panBy(delta.x, delta.y);
					requestPreview();
				}
			}

			const float wheel = GetMouseWheelMove();
			if (wheel != 0.0f) {
				const double factor = std::pow(kZoomFactor, -static_cast<double>(wheel));
				if (view.zoomAt(factor, mouse.x, mouse.y)) {
					requestPreview();
				}
			}

			const bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

			// Shift со стрелками меняет разрешение PNG и не двигает камеру
			if (shiftDown && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
				const int delta = IsKeyPressed(KEY_RIGHT) ? 1 : -1;
				exportIndex = adjustExportPresetIndex(exportIndex, delta);
				settings.exportPreset = kExportPresets[exportIndex].label;
				if (!saveSettings(settings, settingsFile))
					TraceLog(LOG_WARNING, "Could not save settings: %s", settingsFile.string().c_str());
			}

			if (!shiftDown) {
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
				}
			}

			const bool keyboardZoomIn = IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD);
			const bool keyboardZoomOut = IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT);
			if (keyboardZoomIn || keyboardZoomOut) {
				const double factor = keyboardZoomIn ? 1.0 / kZoomFactor : kZoomFactor;
				const double centerX = view.getWidth() * 0.5;
				const double centerY = view.getHeight() * 0.5;
				if (view.zoomAt(factor, centerX, centerY)) {
					requestPreview();
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
				}
				iterationRepeatTime = GetTime() + kIterationRepeatDelay;
			}

			if (IsKeyPressed(KEY_P)) {
				colormap.nextPalette();
				settings.palette = colormap.paletteName();
				if (!saveSettings(settings, settingsFile))
					TraceLog(LOG_WARNING, "Could not save settings: %s", settingsFile.string().c_str());
				if (!data.empty()) {
					renderer.draw(data, colormap, maxIter, dataWidth, dataHeight);
				}
			}

			if (IsKeyPressed(KEY_C)) {
				// Итерации считаем по текущему виду, иначе в буфер попадёт
				// число от прошлого кадра
				const int clipboardIterations = iterationsForView(view.getSpan(), iterationBonus);
				SetClipboardText(formatViewSnapshot(view, clipboardIterations, colormap.paletteName()).c_str());
				notice = ui->copied;
				noticeTime = GetTime();
			}

			if (IsKeyPressed(KEY_V)) {
				const char* clipboard = GetClipboardText();
				ViewSnapshot snapshot;
				if (clipboard == nullptr || !parseViewSnapshot(clipboard, snapshot)) {
					notice = ui->clipboardNoView;
					noticeTime = GetTime();
				} else {
					ColorMap pastedPalette = colormap;
					if (!pastedPalette.setPalette(snapshot.palette)) {
						notice = ui->unknownPalette;
						noticeTime = GetTime();
					} else if (snapshot.iterations < kMinIterations || snapshot.iterations > kMaxIterations) {
						notice = ui->cannotRestoreIterations;
						noticeTime = GetTime();
					} else {
						const View pastedView(snapshot.centerX, snapshot.centerY, snapshot.span, view.getWidth(),
											  view.getHeight());
						const int bonus = std::clamp(snapshot.iterations - iterationsForSpan(pastedView.getSpan()),
													 kMinIterationBonus, kMaxIterationBonus);
						if (iterationsForView(pastedView.getSpan(), bonus) != snapshot.iterations) {
							notice = ui->cannotRestoreIterations;
							noticeTime = GetTime();
						} else {
							view = pastedView;
							iterationBonus = bonus;
							colormap = pastedPalette;
							settings.palette = colormap.paletteName();
							if (!saveSettings(settings, settingsFile))
								TraceLog(LOG_WARNING, "Could not save settings: %s", settingsFile.string().c_str());
							requestFull();
							notice = ui->pastedView;
							noticeTime = GetTime();
						}
					}
				}
			}

			if (IsKeyPressed(KEY_R)) {
				const View home = homeView(fractal.type, view.getWidth(), view.getHeight());
				const bool atHome = sameView(view, home) && iterationBonus == 0;
				const bool homeRequested = hasRequest && sameView(lastRequestedView, home) && lastRequestedBonus == 0;
				if (!atHome || !homeRequested) {
					view = home;
					iterationBonus = 0;
					requestFull();
				}
			}

			if (IsKeyPressed(KEY_H)) {
				showHelp = !showHelp;
			}

			if (IsKeyPressed(KEY_T)) {
				activeJuliaSlider = -1;
				suppressPan = false;
				fractal.type = nextFractalType(fractal.type);
				renderer.setFractalType(fractal.type);
				settings.fractal = fractal;
				if (!saveSettings(settings, settingsFile))
					TraceLog(LOG_WARNING, "Could not save settings: %s", settingsFile.string().c_str());
				requestFull();
			}

			if (IsKeyPressed(KEY_L)) {
				language = nextLanguage(language);
				settings.language = language;
				if (!saveSettings(settings, settingsFile))
					TraceLog(LOG_WARNING, "Could not save settings: %s", settingsFile.string().c_str());
				ui = &uiText(language);
				renderer.setLanguage(language);
				notice.clear();
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
				renderer.draw(data, colormap, maxIter, dataWidth, dataHeight);
				lastFrameTime = GetTime();

				// Устаревший кадр мог прийти после смены вида или размера окна
				if (result.id == lastFullRequestId)
					fullRequested = false;
			}

			ImageExportResult exportResult;
			if (exportWorker.tryTake(exportResult)) {
				if (exportResult.id == saveRequestId) {
					notice = exportResult.saved ? ui->savedPrefix + exportResult.path
												: ui->saveFailedPrefix + exportResult.path;
					noticeTime = GetTime();
				}
				saveRequestId = 0;
			}

			if (IsKeyPressed(KEY_F2)) {
				requestPresetExport();
			}

			const ComplexPoint cursor = view.pointAt(mouse.x, mouse.y);
			std::string info =
				makeInfo(view, maxIter, colormap.paletteName(), renderMs, kExportPresets[exportIndex].label, *ui);

			const bool busy = worker.isBusy();
			std::string status;
			if (busy)
				status = ui->statusRendering;
			if (saveRequestId != 0) {
				if (!status.empty())
					status += " + ";
				status += "PNG " + saveLabel;
			}

			const bool wideWindow = view.getWidth() >= kWideWindowWidth;
			std::string details = makeDetails(view, cursor, wideWindow, *ui);
			if (lastFrameTime >= 0.0 && (busy || previewActive) && wideWindow) {
				char buffer[64];
				std::snprintf(buffer, sizeof(buffer), ui->imageAgeFormat, (GetTime() - lastFrameTime) * 1000.0);
				details += buffer;
			}

			std::string visibleNotice;
			if (!notice.empty() && GetTime() - noticeTime < 3.0) {
				visibleNotice = notice;
			}

			// На узком окне панель подсказок перекрыла бы ползунки Julia
			const bool showHelpNow = showHelp && (fractal.type != FractalType::Julia || view.getWidth() >= 800);
			const JuliaControls juliaControls{fractal.type == FractalType::Julia, fractal.juliaReal, fractal.juliaImag,
											  activeJuliaSlider};
			renderer.display(info, details, visibleNotice, status, showHelpNow, juliaControls);
		}

		// Сохраняем и на выходе: c мог измениться, но кнопку мыши ещё не отпустили
		settings.fractal = fractal;
		if (!saveSettings(settings, settingsFile))
			TraceLog(LOG_WARNING, "Could not save settings: %s", settingsFile.string().c_str());

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

	std::printf("Rendering %dx%d, %d iterations, %s, palette %s...\n", options.settings.width, options.settings.height,
				options.settings.maxIterations, fractalTypeName(options.settings.fractal.type), colormap.paletteName());
	std::fflush(stdout);

	if (!saveFractalImage(options.output, options.settings, colormap)) {
		std::fprintf(stderr, "Could not save %s\n", options.output.c_str());
		return 1;
	}

	std::printf("Saved %s\n", options.output.c_str());
	return 0;
}

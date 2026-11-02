#include "Application.h"

#include "ColorMap.h"
#include "ExportPresets.h"
#include "FractalRender.h"
#include "FractalType.h"
#include "ImageExportWorker.h"
#include "Iterations.h"
#include "Localization.h"
#include "RenderSettings.h"
#include "RenderWorker.h"
#include "Settings.h"
#include "View.h"
#include "ViewSnapshot.h"

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {
	constexpr int kWindowWidth = 1200;
	constexpr int kWindowHeight = 800;
	constexpr double kJuliaInitialSpan = 4.0;
	constexpr double kBurningShipInitialCenterX = -0.5;
	constexpr double kBurningShipInitialCenterY = 0.5;
	constexpr double kBurningShipInitialSpan = 4.0;

	constexpr double kIterationRepeatDelay = 0.15;

	constexpr int kPreviewScale = 2;
	constexpr double kFullRenderDelay = 0.2;
	constexpr double kKeyboardPanSpeed = 720.0;
	constexpr double kMaxFrameTime = 0.05;
	constexpr double kZoomFactor = 1.2;
	// На узком окне длинные строки с координатами не помещаются
	constexpr int kWideWindowWidth = 900;

	std::string screenshotPath(const char* prefix, const char* suffix) {
		const std::time_t now = std::time(nullptr);

		std::tm local{};
#if defined(_WIN32)
		const bool hasTime = localtime_s(&local, &now) == 0;
#else
		const bool hasTime = localtime_r(&now, &local) != nullptr;
#endif

		std::string path = prefix;
		if (hasTime) {
			char stamp[32];
			if (std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &local) > 0) {
				path += '_';
				path += stamp;
			}
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

	// Начальный вид: Julia в нуле, Burning Ship около (-0.5, 0.5)
	View initialView(FractalType type, int width, int height) {
		switch (type) {
			case FractalType::Julia:
				return View(0.0, 0.0, kJuliaInitialSpan, width, height);
			case FractalType::BurningShip:
				return View(kBurningShipInitialCenterX, kBurningShipInitialCenterY, kBurningShipInitialSpan, width,
							height);
			case FractalType::Mandelbrot:
				break;
		}
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

	ColorMap makeColormap(const std::string& name) {
		ColorMap colormap;
		colormap.setPalette(name);
		return colormap;
	}

	int totalThreadCount() {
		const unsigned int hardwareThreads = std::max(1u, std::thread::hardware_concurrency());
		return std::max(2, static_cast<int>(hardwareThreads) - 1);
	}

	int exportThreadCount() {
		return std::max(1, totalThreadCount() / 4);
	}

	int displayThreadCount() {
		return std::max(1, totalThreadCount() - exportThreadCount());
	}
} // namespace

Application::Application()
	: settingsFile(settingsPath()), settings(loadSettings(settingsFile, legacySettingsPath())),
	  language(settings.language), fractal(settings.fractal),
	  renderer(kWindowWidth, kWindowHeight, fractalTitle(fractal.type, language)),
	  colormap(makeColormap(settings.palette)),
	  view(initialView(fractal.type, renderer.getWidth(), renderer.getHeight())), renderWorker(displayThreadCount()),
	  exportWorker(exportThreadCount()), ui(&uiText(language)), renderState(view),
	  exportState(exportPresetIndex(settings.exportPreset)) {
	renderer.setLanguage(language);
	renderer.setFractalType(fractal.type);
	settings.palette = colormap.paletteName();
	renderState.maxIter = iterationsForView(view.getSpan(), iterationBonus);
	setWindowIcon();
}

int Application::run() {
	requestFull();

	while (!WindowShouldClose()) {
		handleResize();
		handleMouseInput();
		handleKeyboardInput();
		updateRenderRequests();
		pollWorkers();
		handleExportKey();
		drawFrame();
	}

	persistSettings();
	return 0;
}

void Application::handleResize() {
	// Окно могли растянуть или свернуть. Нулевой размер бывает у свернутого окна, такой кадр не запрашиваем.
	const int screenWidth = GetScreenWidth();
	const int screenHeight = GetScreenHeight();
	if (screenWidth > 0 && screenHeight > 0 && (screenWidth != view.getWidth() || screenHeight != view.getHeight())) {
		renderer.resize(screenWidth, screenHeight);
		view.resize(screenWidth, screenHeight);
		requestPreview();
	}
}

void Application::handleMouseInput() {
	const Vector2 mouse = GetMousePosition();
	handleJuliaControls(mouse.x, mouse.y);
	handlePan();
	handleZoom(mouse.x, mouse.y);
}

void Application::handleJuliaControls(float mouseX, float mouseY) {
	// Ползунки c показываются только у Julia
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		uiState.suppressPan = fractal.type == FractalType::Julia && renderer.juliaPanelAt(mouseX, mouseY);
		if (fractal.type == FractalType::Julia) {
			const int slider = renderer.juliaSliderAt(mouseX, mouseY);
			if (slider >= 0) {
				uiState.activeJuliaSlider = slider;
				applyJuliaSlider(slider, mouseX);
			}
		}
	}

	if (uiState.activeJuliaSlider >= 0) {
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			applyJuliaSlider(uiState.activeJuliaSlider, mouseX);
		} else {
			// Кнопку отпустили или окно потеряло фокус
			uiState.activeJuliaSlider = -1;
			persistSettings();
			requestFull();
		}
	}

	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		uiState.suppressPan = false;
}

void Application::handlePan() {
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && uiState.activeJuliaSlider < 0 && !uiState.suppressPan) {
		const Vector2 delta = GetMouseDelta();
		if (delta.x != 0.0f || delta.y != 0.0f) {
			view.panBy(delta.x, delta.y);
			requestPreview();
		}
	}
}

void Application::handleZoom(float mouseX, float mouseY) {
	const float wheel = GetMouseWheelMove();
	if (wheel != 0.0f) {
		const double factor = std::pow(kZoomFactor, -static_cast<double>(wheel));
		if (view.zoomAt(factor, mouseX, mouseY))
			requestPreview();
	}
}

void Application::handleKeyboardInput() {
	const bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

	handleExportPresetKeys(shiftDown);
	handleNavigationKeys(shiftDown);
	handleIterationKeys();
	handlePaletteKey();
	handleClipboardKeys();
	handleViewKeys();
	handleModeKeys();
}

void Application::handleExportPresetKeys(bool shiftDown) {
	// Shift со стрелками меняет разрешение PNG и не двигает камеру
	if (shiftDown && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
		const int delta = IsKeyPressed(KEY_RIGHT) ? 1 : -1;
		exportState.index = adjustExportPresetIndex(exportState.index, delta);
		settings.exportPreset = kExportPresets[exportState.index].label;
		persistSettings();
	}
}

void Application::handleNavigationKeys(bool shiftDown) {
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
		if (view.zoomAt(factor, centerX, centerY))
			requestPreview();
	}
}

void Application::handleIterationKeys() {
	int iterationDelta = 0;
	if (IsKeyDown(KEY_RIGHT_BRACKET))
		iterationDelta += kIterationStep;
	if (IsKeyDown(KEY_LEFT_BRACKET))
		iterationDelta -= kIterationStep;

	// Первое нажатие срабатывает сразу, дальше шаг повторяется, пока клавиша удерживается
	const bool iterationPressed = IsKeyPressed(KEY_LEFT_BRACKET) || IsKeyPressed(KEY_RIGHT_BRACKET);
	if (iterationDelta != 0 && (iterationPressed || GetTime() >= iterationRepeatTime)) {
		int nextBonus = 0;
		if (nextIterationBonus(view.getSpan(), iterationBonus, iterationDelta, nextBonus)) {
			iterationBonus = nextBonus;
			requestFull();
		}
		iterationRepeatTime = GetTime() + kIterationRepeatDelay;
	}
}

void Application::handlePaletteKey() {
	if (!IsKeyPressed(KEY_P))
		return;

	colormap.nextPalette();
	settings.palette = colormap.paletteName();
	persistSettings();
	if (!renderState.data.empty()) {
		renderer.draw(renderState.data, colormap, renderState.maxIter, renderState.dataWidth, renderState.dataHeight);
	}
}

void Application::handleClipboardKeys() {
	if (IsKeyPressed(KEY_C)) {
		// Итерации по текущему виду, иначе в буфер попадёт число от прошлого кадра
		const int clipboardIterations = iterationsForView(view.getSpan(), iterationBonus);
		SetClipboardText(formatViewSnapshot(view, clipboardIterations, colormap.paletteName()).c_str());
		uiState.notice = ui->copied;
		uiState.noticeTime = GetTime();
	}

	if (IsKeyPressed(KEY_V)) {
		const char* clipboard = GetClipboardText();
		ViewSnapshot snapshot;
		if (clipboard == nullptr || !parseViewSnapshot(clipboard, snapshot)) {
			uiState.notice = ui->clipboardNoView;
			uiState.noticeTime = GetTime();
		} else {
			ColorMap pastedPalette = colormap;
			if (!pastedPalette.setPalette(snapshot.palette)) {
				uiState.notice = ui->unknownPalette;
				uiState.noticeTime = GetTime();
			} else if (snapshot.iterations < kMinIterations || snapshot.iterations > kMaxIterations) {
				uiState.notice = ui->cannotRestoreIterations;
				uiState.noticeTime = GetTime();
			} else {
				const View pastedView(snapshot.centerX, snapshot.centerY, snapshot.span, view.getWidth(),
									  view.getHeight());
				const int bonus = std::clamp(snapshot.iterations - iterationsForSpan(pastedView.getSpan()),
											 kMinIterationBonus, kMaxIterationBonus);
				if (iterationsForView(pastedView.getSpan(), bonus) != snapshot.iterations) {
					uiState.notice = ui->cannotRestoreIterations;
					uiState.noticeTime = GetTime();
				} else {
					view = pastedView;
					iterationBonus = bonus;
					colormap = pastedPalette;
					settings.palette = colormap.paletteName();
					persistSettings();
					requestFull();
					uiState.notice = ui->pastedView;
					uiState.noticeTime = GetTime();
				}
			}
		}
	}
}

void Application::handleViewKeys() {
	if (IsKeyPressed(KEY_R)) {
		const View initial = initialView(fractal.type, view.getWidth(), view.getHeight());
		const bool atInitial = sameView(view, initial) && iterationBonus == 0;
		const bool initialRequested = renderState.hasRequest && sameView(renderState.lastRequestedView, initial) &&
									  renderState.lastRequestedBonus == 0;
		if (!atInitial || !initialRequested) {
			view = initial;
			iterationBonus = 0;
			requestFull();
		}
	}

	if (IsKeyPressed(KEY_H))
		uiState.showHelp = !uiState.showHelp;
}

void Application::handleModeKeys() {
	if (IsKeyPressed(KEY_T)) {
		uiState.activeJuliaSlider = -1;
		uiState.suppressPan = false;
		fractal.type = nextFractalType(fractal.type);
		renderer.setFractalType(fractal.type);
		persistSettings();
		requestFull();
	}

	if (IsKeyPressed(KEY_L)) {
		language = nextLanguage(language);
		settings.language = language;
		persistSettings();
		ui = &uiText(language);
		renderer.setLanguage(language);
		uiState.notice.clear();
	}
}

void Application::handleExportKey() {
	if (IsKeyPressed(KEY_F2))
		requestPresetExport();
}

void Application::updateRenderRequests() {
	// После остановки ввода заменяем предпросмотр полным кадром
	if (renderState.previewActive && !renderState.fullRequested &&
		GetTime() - renderState.lastInputTime >= kFullRenderDelay) {
		requestFull();
	}
}

void Application::pollWorkers() {
	handleRenderResult();
	handleExportResult();
}

void Application::handleRenderResult() {
	// Пока воркер считает новый кадр, на экране остается предыдущий
	RenderResult result;
	if (!renderWorker.tryTake(result))
		return;

	renderState.data = std::move(result.data);
	renderState.dataWidth = result.width;
	renderState.dataHeight = result.height;
	renderState.maxIter = result.maxIterations;
	renderState.renderMs = result.renderMs;
	renderer.draw(renderState.data, colormap, renderState.maxIter, renderState.dataWidth, renderState.dataHeight);
	renderState.lastFrameTime = GetTime();

	// Устаревший кадр мог прийти после смены вида или размера окна
	if (result.id == renderState.lastFullRequestId)
		renderState.fullRequested = false;
}

void Application::handleExportResult() {
	ImageExportResult exportResult;
	if (!exportWorker.tryTake(exportResult))
		return;

	if (exportResult.id == exportState.requestId) {
		uiState.notice =
			exportResult.saved ? ui->savedPrefix + exportResult.path : ui->saveFailedPrefix + exportResult.path;
		uiState.noticeTime = GetTime();
	}
	exportState.requestId = 0;
}

RenderSettings Application::makeRenderSettings(int width, int height, double span) const {
	RenderSettings settings;
	settings.width = width;
	settings.height = height;
	// Итерации по экранному span: экспорт шире окна, но детали центра не должны теряться.
	settings.maxIterations = iterationsForView(view.getSpan(), iterationBonus);
	settings.fractal = fractal;
	settings.centerX = view.getCenterX();
	settings.centerY = view.getCenterY();
	settings.span = span;
	return settings;
}

void Application::submitRender(int width, int height, bool full) {
	RenderRequest request;
	request.id = renderState.nextRequestId++;
	request.settings = makeRenderSettings(width, height, view.getSpan());
	renderWorker.request(request);

	if (full)
		renderState.lastFullRequestId = request.id;

	renderState.lastRequestedView = view;
	renderState.lastRequestedBonus = iterationBonus;
	renderState.hasRequest = true;
}

void Application::requestPreview() {
	submitRender(std::max(view.getWidth() / kPreviewScale, 1), std::max(view.getHeight() / kPreviewScale, 1), false);
	// Предпросмотр относится к новому виду: старый полный кадр больше не актуален
	renderState.lastFullRequestId = 0;
	renderState.previewActive = true;
	renderState.fullRequested = false;
	renderState.lastInputTime = GetTime();
}

void Application::requestFull() {
	submitRender(view.getWidth(), view.getHeight(), true);
	renderState.previewActive = false;
	renderState.fullRequested = true;
	renderState.lastInputTime = GetTime();
}

void Application::requestPresetExport() {
	if (exportState.requestId != 0) {
		uiState.notice = ui->saveInProgress;
		uiState.noticeTime = GetTime();
		return;
	}

	const ExportPreset& preset = kExportPresets[exportState.index];
	const double exportSpan = exportSpanFor(view.getSpan(), view.getWidth(), view.getHeight(), preset);

	ImageExportRequest request;
	request.id = renderState.nextRequestId++;
	request.settings = makeRenderSettings(preset.width, preset.height, exportSpan);
	request.path = screenshotPath(fractalTypeName(fractal.type), preset.suffix);
	request.colormap = colormap;

	exportState.requestId = request.id;
	exportState.label = preset.label;
	exportWorker.request(request);

	char buffer[64];
	std::snprintf(buffer, sizeof(buffer), ui->renderingExportFormat, preset.label);
	uiState.notice = buffer;
	uiState.noticeTime = GetTime();
}

void Application::applyJuliaSlider(int slider, float mouseX) {
	const double value = renderer.juliaSliderValue(slider, mouseX);
	double& target = slider == 0 ? fractal.juliaReal : fractal.juliaImag;
	if (value != target) {
		target = value;
		requestPreview();
	}
}

void Application::drawFrame() {
	const Vector2 mouse = GetMousePosition();
	const ComplexPoint cursor = view.pointAt(mouse.x, mouse.y);
	const bool busy = renderWorker.isBusy();

	const std::string info = buildInfo();
	const std::string details = buildDetails(cursor, busy);
	const std::string status = buildStatus(busy);
	const std::string visibleNotice = buildNotice();

	// На узком окне панель подсказок перекрыла бы ползунки Julia
	const bool showHelpNow = uiState.showHelp && (fractal.type != FractalType::Julia || view.getWidth() >= 800);
	const JuliaControls juliaControls{fractal.type == FractalType::Julia, uiState.activeJuliaSlider};
	renderer.display(info, details, visibleNotice, status, showHelpNow, juliaControls, fractal);
}

std::string Application::buildInfo() const {
	return makeInfo(view, renderState.maxIter, colormap.paletteName(), renderState.renderMs,
					kExportPresets[exportState.index].label, *ui);
}

std::string Application::buildDetails(const ComplexPoint& cursor, bool busy) const {
	const bool wideWindow = view.getWidth() >= kWideWindowWidth;
	std::string details = makeDetails(view, cursor, wideWindow, *ui);
	if (renderState.lastFrameTime >= 0.0 && (busy || renderState.previewActive) && wideWindow) {
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), ui->imageAgeFormat, (GetTime() - renderState.lastFrameTime) * 1000.0);
		details += buffer;
	}
	return details;
}

std::string Application::buildStatus(bool busy) const {
	std::string status;
	if (busy)
		status = ui->statusRendering;
	if (exportState.requestId != 0) {
		if (!status.empty())
			status += " + ";
		status += "PNG " + exportState.label;
	}
	return status;
}

std::string Application::buildNotice() const {
	if (!uiState.notice.empty() && GetTime() - uiState.noticeTime < 3.0)
		return uiState.notice;
	return {};
}

void Application::persistSettings() {
	settings.fractal = fractal;
	if (!saveSettings(settings, settingsFile))
		TraceLog(LOG_WARNING, "Could not save settings: %s", settingsFile.string().c_str());
}

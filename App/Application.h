#pragma once

#include "ColorMap.h"
#include "FractalRender.h"
#include "FractalType.h"
#include "ImageExportWorker.h"
#include "Localization.h"
#include "RenderWorker.h"
#include "Settings.h"
#include "View.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

/// Приложение с интерактивным окном.
class Application {
  public:
	Application();
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	/// Запускает главный цикл и возвращает код выхода.
	int run();

  private:
	struct RenderState {
		explicit RenderState(const View& initialView) : lastRequestedView(initialView) {
		}

		std::vector<double> data;
		int dataWidth = 0;
		int dataHeight = 0;
		int maxIter = 0;
		double renderMs = 0.0;
		double lastFrameTime = -1.0;
		bool previewActive = false;
		bool fullRequested = false;
		double lastInputTime = 0.0;
		bool hasRequest = false;
		View lastRequestedView;
		int lastRequestedBonus = 0;
		std::uint64_t nextRequestId = 1;
		std::uint64_t lastFullRequestId = 0;
	};

	struct ExportState {
		explicit ExportState(int presetIndex) : index(presetIndex) {
		}

		int index = 0;
		std::uint64_t requestId = 0;
		std::string label;
	};

	struct UiState {
		bool showHelp = true;
		int activeJuliaSlider = -1;
		bool suppressPan = false;
		std::string notice;
		double noticeTime = 0.0;
	};

	void handleResize();
	void handleMouseInput();
	void handleJuliaControls(float mouseX, float mouseY);
	void handlePan();
	void handleZoom(float mouseX, float mouseY);

	void handleKeyboardInput();
	void handleExportPresetKeys(bool shiftDown);
	void handleNavigationKeys(bool shiftDown);
	void handleIterationKeys();
	void handlePaletteKey();
	void handleClipboardKeys();
	void handleViewKeys();
	void handleModeKeys();
	void handleExportKey();

	void updateRenderRequests();
	void pollWorkers();
	void handleRenderResult();
	void handleExportResult();

	void submitRender(int width, int height, bool full);
	void requestPreview();
	void requestFull();
	void requestPresetExport();
	void applyJuliaSlider(int slider, float mouseX);

	void drawFrame();
	std::string buildInfo() const;
	std::string buildDetails(const ComplexPoint& cursor, bool busy) const;
	std::string buildStatus(bool busy) const;
	std::string buildNotice() const;
	void persistSettings();

	std::filesystem::path settingsFile;
	Settings settings;
	Language language;
	FractalParams fractal;
	FractalRender renderer;
	ColorMap colormap;
	View view;
	RenderWorker renderWorker;
	ImageExportWorker exportWorker;
	const UiText* ui = nullptr;
	int iterationBonus = 0;
	double iterationRepeatTime = 0.0;
	RenderState renderState;
	ExportState exportState;
	UiState uiState;
};

#pragma once

#include "ColorMap.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>

// Параметры экспорта одного PNG.
struct ImageExportRequest {
	std::uint64_t id = 0;
	int width = 0;
	int height = 0;
	int maxIterations = 0;
	double centerX = 0.0;
	double centerY = 0.0;
	double span = 0.0;
	std::string path;
	ColorMap colormap;
};

// Результат экспорта: путь и признак успешной записи.
struct ImageExportResult {
	std::uint64_t id = 0;
	std::string path;
	bool saved = false;
};

// Считает кадр и сохраняет PNG в фоновом потоке, чтобы главный поток
// не блокировался на цветокоррекции и кодировании файла.
class ImageExportWorker {
  public:
	// threadLimit ограничивает число потоков OpenMP; 0 оставляет
	// главному потоку два ядра.
	explicit ImageExportWorker(int threadLimit = 0);
	~ImageExportWorker();

	ImageExportWorker(const ImageExportWorker&) = delete;
	ImageExportWorker& operator=(const ImageExportWorker&) = delete;

	// Ставит последний запрос в очередь. Текущий экспорт доходит до конца.
	void request(const ImageExportRequest& request);

	// Забирает готовый результат, если он есть. Не блокирует.
	bool tryTake(ImageExportResult& result);

	// Есть ли активная или ожидающая задача.
	bool isBusy() const;

  private:
	void run(std::stop_token stopToken);

	mutable std::mutex mutex;
	std::condition_variable cv;
	std::optional<ImageExportRequest> pending;
	std::optional<ImageExportResult> finished;
	std::atomic<bool> busy{false};
	bool stopping = false;
	int threadLimit = 0;
	std::jthread worker;
};

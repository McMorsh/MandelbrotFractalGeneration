#include "ImageExportWorker.h"

#include "Fractal.h"
#include "ImageRender.h"

#include <omp.h>

#include <algorithm>
#include <utility>
#include <vector>

ImageExportWorker::ImageExportWorker(int threadLimit)
	: threadLimit(threadLimit), worker([this](std::stop_token stopToken) { run(stopToken); }) {
}

ImageExportWorker::~ImageExportWorker() {
	{
		std::lock_guard lock(mutex);
		stopping = true;
	}

	worker.request_stop();
	cv.notify_all();

	if (worker.joinable())
		worker.join();
}

void ImageExportWorker::request(const ImageExportRequest& request) {
	{
		std::lock_guard lock(mutex);
		if (stopping)
			return;
		pending = request;
	}

	cv.notify_one();
}

bool ImageExportWorker::tryTake(ImageExportResult& result) {
	std::lock_guard lock(mutex);
	if (!finished)
		return false;

	result = std::move(*finished);
	finished.reset();
	return true;
}

bool ImageExportWorker::isBusy() const {
	std::lock_guard lock(mutex);
	return busy.load(std::memory_order_relaxed) || pending.has_value();
}

void ImageExportWorker::run(std::stop_token stopToken) {
	const int threads = threadLimit > 0 ? threadLimit : std::max(1, omp_get_num_procs() - 2);
	omp_set_num_threads(threads);

	while (true) {
		ImageExportRequest request;

		{
			std::unique_lock lock(mutex);
			cv.wait(lock, [this] { return stopping || pending.has_value(); });
			if (stopping)
				return;

			request = *pending;
			pending.reset();
		}

		busy.store(true, std::memory_order_relaxed);

		const Fractal fractal(request.maxIterations, request.fractal);
		const std::vector<double> data =
			fractal.generate(request.width, request.height, request.centerX, request.centerY, request.span, stopToken);

		bool saved = false;
		if (!stopToken.stop_requested() && !data.empty()) {
			saved = saveIterationImage(request.path, data, request.width, request.height, request.maxIterations,
									   request.colormap);
		}

		busy.store(false, std::memory_order_relaxed);

		if (stopToken.stop_requested())
			return;

		{
			std::lock_guard lock(mutex);
			if (stopping)
				return;
			finished = ImageExportResult{request.id, request.path, saved};
		}
	}
}

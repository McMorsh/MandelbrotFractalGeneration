#include "RenderWorker.h"
#include "Fractal.h"

#include <omp.h>

#include <algorithm>
#include <chrono>
#include <utility>

RenderWorker::RenderWorker() : worker([this](std::stop_token stopToken) { run(stopToken); }) {
}

RenderWorker::~RenderWorker() {
	{
		std::lock_guard lock(mutex);
		stopping = true;
	}

	worker.request_stop();
	cv.notify_all();

	if (worker.joinable())
		worker.join();
}

void RenderWorker::request(const RenderRequest& request) {
	{
		std::lock_guard lock(mutex);
		if (stopping)
			return;
		pending = request;
	}

	cv.notify_one();
}

bool RenderWorker::tryTake(RenderResult& result) {
	std::lock_guard lock(mutex);
	if (!finished)
		return false;

	result = std::move(*finished);
	finished.reset();
	return true;
}

bool RenderWorker::isBusy() const {
	std::lock_guard lock(mutex);
	return busy.load(std::memory_order_relaxed) || pending.has_value();
}

void RenderWorker::run(std::stop_token stopToken) {
	// Оставляем ядра главному потоку, иначе окно теряет кадры при перетаскивании
	omp_set_num_threads(std::max(1, omp_get_num_procs() - 2));

	while (true) {
		RenderRequest request;

		{
			std::unique_lock lock(mutex);
			cv.wait(lock, [this] { return stopping || pending.has_value(); });
			if (stopping)
				return;

			request = *pending;
			pending.reset();
		}

		busy.store(true, std::memory_order_relaxed);

		const Fractal fractal(request.maxIterations);
		const auto start = std::chrono::steady_clock::now();
		std::vector<double> data =
			fractal.generate(request.width, request.height, request.centerX, request.centerY, request.span, stopToken);
		const double renderMs =
			std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

		busy.store(false, std::memory_order_relaxed);

		if (stopToken.stop_requested())
			return;
		if (data.empty())
			continue;

		{
			std::lock_guard lock(mutex);
			if (stopping)
				return;
			finished = RenderResult{request.width, request.height, request.maxIterations, renderMs, std::move(data)};
		}
	}
}

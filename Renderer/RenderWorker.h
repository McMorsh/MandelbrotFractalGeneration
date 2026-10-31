#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>
#include <vector>

// Параметры одного кадра.
struct RenderRequest {
	// Идентификатор запроса: по нему отбрасываются устаревшие кадры
	std::uint64_t id = 0;
	int width = 0;
	int height = 0;
	int maxIterations = 0;
	double centerX = 0.0;
	double centerY = 0.0;
	double span = 0.0;
};

// Готовый кадр и время его расчёта.
struct RenderResult {
	std::uint64_t id = 0;
	int width = 0;
	int height = 0;
	int maxIterations = 0;
	double renderMs = 0.0;
	std::vector<double> data;
};

// Считает кадры в фоновом потоке. Новый запрос заменяет ожидающий,
// но не прерывает уже начатый расчёт.
class RenderWorker {
  public:
	RenderWorker();
	~RenderWorker();

	RenderWorker(const RenderWorker&) = delete;
	RenderWorker& operator=(const RenderWorker&) = delete;

	// Ставит последний запрос в очередь. Текущий расчёт доходит до конца.
	void request(const RenderRequest& request);

	// Забирает готовый кадр, если он есть. Не блокирует.
	bool tryTake(RenderResult& result);

	// Есть ли активный или ожидающий расчёт.
	bool isBusy() const;

  private:
	void run(std::stop_token stopToken);

	mutable std::mutex mutex;
	std::condition_variable cv;
	std::optional<RenderRequest> pending;
	std::optional<RenderResult> finished;
	std::atomic<bool> busy{false};
	bool stopping = false;
	std::jthread worker;
};

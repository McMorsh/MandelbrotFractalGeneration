#include "render_worker.hpp"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <stop_token>
#include <utility>
#include <vector>

#include <omp.h>

#include "fractal.hpp"

RenderWorker::RenderWorker(int thread_limit)
    : thread_limit_(thread_limit),
      worker_([this](std::stop_token stop_token) { Run(stop_token); }) {}

RenderWorker::~RenderWorker() {
  {
    std::lock_guard lock(mutex_);
    stopping_ = true;
  }

  worker_.request_stop();
  cv_.notify_all();

  if (worker_.joinable()) {
    worker_.join();
  }
}

void RenderWorker::Request(const RenderRequest& request) {
  {
    std::lock_guard lock(mutex_);
    if (stopping_) {
      return;
    }
    pending_ = request;
  }

  cv_.notify_one();
}

bool RenderWorker::TryTake(RenderResult& result) {
  std::lock_guard lock(mutex_);
  if (!finished_) {
    return false;
  }

  result = std::move(*finished_);
  finished_.reset();
  return true;
}

bool RenderWorker::IsBusy() const {
  std::lock_guard lock(mutex_);
  return busy_.load(std::memory_order_relaxed) || pending_.has_value();
}

void RenderWorker::Run(std::stop_token stop_token) {
  // Leave cores for the main thread unless a limit is set explicitly.
  const int threads =
      thread_limit_ > 0 ? thread_limit_ : std::max(1, omp_get_num_procs() - 2);
  omp_set_num_threads(threads);

  while (true) {
    RenderRequest request;

    {
      std::unique_lock lock(mutex_);
      cv_.wait(lock, [this] { return stopping_ || pending_.has_value(); });
      if (stopping_) {
        return;
      }

      request = *pending_;
      pending_.reset();
    }

    busy_.store(true, std::memory_order_relaxed);

    const RenderSettings& settings = request.settings;
    const Fractal fractal(settings.max_iterations, settings.fractal);
    const auto start = std::chrono::steady_clock::now();
    std::vector<double> data =
        fractal.Generate(settings.width, settings.height, settings.center_x,
                         settings.center_y, settings.span, stop_token);
    const double render_ms = std::chrono::duration<double, std::milli>(
                                 std::chrono::steady_clock::now() - start)
                                 .count();

    busy_.store(false, std::memory_order_relaxed);

    if (stop_token.stop_requested()) {
      return;
    }
    if (data.empty()) {
      continue;
    }

    {
      std::lock_guard lock(mutex_);
      if (stopping_) {
        return;
      }
      finished_ = RenderResult{request.id,      settings.width,
                               settings.height, settings.max_iterations,
                               render_ms,       std::move(data)};
    }
  }
}

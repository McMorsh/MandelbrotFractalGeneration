#include "image_export_worker.hpp"

#include <algorithm>
#include <mutex>
#include <stop_token>
#include <utility>
#include <vector>

#include <omp.h>

#include "fractal.hpp"
#include "image_render.hpp"

ImageExportWorker::ImageExportWorker(int thread_limit)
    : thread_limit_(thread_limit),
      worker_([this](std::stop_token stop_token) { Run(stop_token); }) {}

ImageExportWorker::~ImageExportWorker() {
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

void ImageExportWorker::Request(const ImageExportRequest& request) {
  {
    std::lock_guard lock(mutex_);
    if (stopping_) {
      return;
    }
    pending_ = request;
  }

  cv_.notify_one();
}

bool ImageExportWorker::TryTake(ImageExportResult& result) {
  std::lock_guard lock(mutex_);
  if (!finished_) {
    return false;
  }

  result = std::move(*finished_);
  finished_.reset();
  return true;
}

bool ImageExportWorker::IsBusy() const {
  std::lock_guard lock(mutex_);
  return busy_.load(std::memory_order_relaxed) || pending_.has_value();
}

void ImageExportWorker::Run(std::stop_token stop_token) {
  const int threads =
      thread_limit_ > 0 ? thread_limit_ : std::max(1, omp_get_num_procs() - 2);
  omp_set_num_threads(threads);

  while (true) {
    ImageExportRequest request;

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
    const std::vector<double> data =
        fractal.Generate(settings.width, settings.height, settings.center_x,
                         settings.center_y, settings.span, stop_token);

    bool saved = false;
    if (!stop_token.stop_requested() && !data.empty()) {
      saved = SaveIterationImage(request.path, data, settings.width,
                                 settings.height, settings.max_iterations,
                                 request.colormap);
    }

    busy_.store(false, std::memory_order_relaxed);

    if (stop_token.stop_requested()) {
      return;
    }

    {
      std::lock_guard lock(mutex_);
      if (stopping_) {
        return;
      }
      finished_ = ImageExportResult{request.id, request.path, saved};
    }
  }
}

#ifndef FRACTALGENERATION_RENDERER_IMAGE_EXPORT_WORKER_HPP_
#define FRACTALGENERATION_RENDERER_IMAGE_EXPORT_WORKER_HPP_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>

#include "color_map.hpp"
#include "render_settings.hpp"

/// Parameters of a single PNG export.
struct ImageExportRequest {
  std::uint64_t id = 0;
  RenderSettings settings;
  std::string path;
  ColorMap colormap;
};

/// Export result: the path and whether writing succeeded.
struct ImageExportResult {
  std::uint64_t id = 0;
  std::string path;
  bool saved = false;
};

/// Renders a frame and saves a PNG in the background so the main thread does
/// not wait for the render and file write.
class ImageExportWorker {
 public:
  /// Creates the worker and starts the background thread.
  /// @param thread_limit limits the number of OpenMP threads; 0 leaves two
  /// cores for the main thread
  explicit ImageExportWorker(int thread_limit = 0);
  /// Stops the worker and joins the background thread.
  ~ImageExportWorker();

  ImageExportWorker(const ImageExportWorker&) = delete;
  ImageExportWorker& operator=(const ImageExportWorker&) = delete;

  /// Queues the latest request. The current export runs to completion.
  void Request(const ImageExportRequest& request);

  /// Takes a finished result if one is available. Does not block.
  /// @param result receives the result on success
  /// @return false if no result is ready.
  bool TryTake(ImageExportResult& result);

  /// Reports whether a task is active or pending.
  bool IsBusy() const;

 private:
  void Run(std::stop_token stop_token);

  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::optional<ImageExportRequest> pending_;
  std::optional<ImageExportResult> finished_;
  std::atomic<bool> busy_{false};
  bool stopping_ = false;
  int thread_limit_ = 0;
  std::jthread worker_;
};

#endif  // FRACTALGENERATION_RENDERER_IMAGE_EXPORT_WORKER_HPP_

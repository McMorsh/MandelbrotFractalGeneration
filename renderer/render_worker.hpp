#ifndef FRACTALGENERATION_RENDERER_RENDER_WORKER_HPP_
#define FRACTALGENERATION_RENDERER_RENDER_WORKER_HPP_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>
#include <vector>

#include "render_settings.hpp"

/// Parameters of a single frame.
struct RenderRequest {
  /// Request identifier; it is used to discard stale frames.
  std::uint64_t id = 0;
  RenderSettings settings;
};

/// A finished frame and its render time.
struct RenderResult {
  std::uint64_t id = 0;
  int width = 0;
  int height = 0;
  int max_iterations = 0;
  double render_ms = 0.0;
  std::vector<double> data;
};

/// Renders frames in a background thread. A new request replaces the pending
/// one but does not interrupt an ongoing render.
class RenderWorker {
 public:
  /// Creates the worker and starts the background thread.
  /// @param thread_limit limits the number of OpenMP threads; 0 leaves two
  /// cores for the main thread
  explicit RenderWorker(int thread_limit = 0);
  /// Stops the worker and joins the background thread.
  ~RenderWorker();

  RenderWorker(const RenderWorker&) = delete;
  RenderWorker& operator=(const RenderWorker&) = delete;

  /// Queues the latest request. The current render runs to completion.
  void Request(const RenderRequest& request);

  /// Takes a finished frame if one is available. Does not block.
  /// @param result receives the frame on success
  /// @return false if no frame is ready.
  bool TryTake(RenderResult& result);

  /// Reports whether a render is active or pending.
  bool IsBusy() const;

 private:
  void Run(std::stop_token stop_token);

  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::optional<RenderRequest> pending_;
  std::optional<RenderResult> finished_;
  std::atomic<bool> busy_{false};
  bool stopping_ = false;
  int thread_limit_ = 0;
  std::jthread worker_;
};

#endif  // FRACTALGENERATION_RENDERER_RENDER_WORKER_HPP_

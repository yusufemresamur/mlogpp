#include "src/sink/async_sink.hpp"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace mlogpp {

struct AsyncSinkImpl {
  Sink inner;
  // todo(yusufemresamur): Add fixed size ring buffer for better performance
  std::queue<LogRecord> queue;
  std::size_t const capacity;
  std::mutex mutex;
  // condition_variable_any required for the stop_token wait overload
  std::condition_variable_any cv;
  std::jthread worker;

  explicit AsyncSinkImpl(Sink inner_, std::size_t const capacity)
      : inner(std::move(inner_)),
        capacity(capacity),
        worker([this](std::stop_token const& stoken) { Work(stoken); }) {}

  void Work(std::stop_token const& stoken) {
    while (!stoken.stop_requested()) {
      std::unique_lock lock{mutex};
      // Wakes when queue is non-empty OR stop is requested.
      // Returns false only when stop requested with an empty queue.
      cv.wait(lock, stoken, [this] { return !queue.empty(); });
      while (!queue.empty()) {
        auto record = std::move(queue.front());
        queue.pop();
        lock.unlock();
        inner(record);
        lock.lock();
      }
    }
  }
};

AsyncSink::AsyncSink(Sink inner, std::size_t const capacity)
    : impl_(std::make_shared<AsyncSinkImpl>(std::move(inner), capacity)) {}

void AsyncSink::operator()(LogRecord const& record) {
  bool notify = false;
  {
    std::scoped_lock lock{impl_->mutex};
    if (impl_->queue.size() < impl_->capacity) {
      impl_->queue.push(record);
      notify = true;
    }
  }
  if (notify) impl_->cv.notify_one();
}

}  // namespace mlogpp

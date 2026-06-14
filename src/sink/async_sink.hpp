#pragma once
#ifndef MLOGPP_ASYNC_SINK_HPP_
#define MLOGPP_ASYNC_SINK_HPP_

#include "src/record.hpp"
#include "src/sink/sink.hpp"
#include <memory>

namespace mlogpp {

struct AsyncSinkImpl;

/**
 * @brief Async wrapper sink. Enqueues log records from calling threads and
 * drains them on a dedicated background jthread, decoupling I/O latency from
 * the callers.
 *
 * Records are dropped silently when the queue is full (non-blocking enqueue).
 * On destruction the background thread is stopped and all queued records are
 * flushed before the thread is joined.
 *
 * @note @c AsyncSink is not copyable. It is movable via the @c shared_ptr to
 * its internal state, which keeps the background thread's captured @c this
 * pointer stable across moves.
 */
class AsyncSink {
 public:
  static constexpr std::size_t kDefaultCapacity = 8192;

  /**
   * @brief Construct an AsyncSink wrapping @p inner.
   *
   * @param inner    Sink to write records to from the background thread.
   * @param capacity Maximum number of records buffered before drops occur.
   */
  explicit AsyncSink(Sink inner, std::size_t const capacity = kDefaultCapacity);

  AsyncSink(AsyncSink const&) = delete;
  AsyncSink& operator=(AsyncSink const&) = delete;
  AsyncSink(AsyncSink&&) noexcept = default;
  AsyncSink& operator=(AsyncSink&&) noexcept = default;
  ~AsyncSink() = default;

  /**
   * @brief Enqueue @p record for async dispatch. Drops the record if the queue
   * is at capacity.
   */
  void operator()(LogRecord const& record);

 private:
  std::shared_ptr<AsyncSinkImpl> impl_;
};

/**
 * @brief Wrap @p inner in an @c AsyncSink. See @c AsyncSink for details.
 *
 * @param inner    Sink to dispatch records to asynchronously.
 * @param capacity Maximum queue depth before records are dropped.
 * @return Sink    AsyncSink wrapped in a Sink type-erasure wrapper.
 */
[[nodiscard]] inline Sink MakeAsyncSink(
    Sink inner, std::size_t const capacity = AsyncSink::kDefaultCapacity) {
  return Sink{AsyncSink{std::move(inner), capacity}};
}

}  // namespace mlogpp

#endif  // MLOGPP_ASYNC_SINK_HPP_

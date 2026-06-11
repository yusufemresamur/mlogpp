#pragma once
#ifndef MLOGPP_SINK_HPP_
#define MLOGPP_SINK_HPP_

#include "src/format/formatter.hpp"
#include "src/record.hpp"
#include <concepts>
#include <memory>
#include <print>

namespace mlogpp {

/**
 * @brief Concept for sink functions: any callable (const LogRecord&) -> void.
 * Allows users to define custom sinks by simply providing a operator() that
 * takes a LogRecord and returns void. This is used in the Sink type-erasure
 * wrapper to allow for flexible sink implementations.
 *
 * @tparam S Sink function type.
 */
template <typename S>
concept SinkFunction = requires(S s, LogRecord const& r) {
  { s(r) } -> std::same_as<void>;
};

/**
 * @brief Type-erased wrapper for sink functions.
 *
 */
class Sink {
 public:
  /**
   * @brief Construct a new Sink from any callable that satisfies the
   * SinkFunction concept.
   *
   * @tparam S Type of the sink function, must satisfy the SinkFunction concept.
   * @param s Sink function to wrap.
   */
  template <SinkFunction S>
    requires(!std::same_as<std::decay_t<S>, Sink>)
  explicit Sink(S&& s)
      : impl_(
            std::make_shared<SinkModel<std::decay_t<S>>>(std::forward<S>(s))) {}

  // Explicitly allow copying and moving of Sink
  Sink(Sink const&) = default;
  Sink& operator=(Sink const&) = default;
  Sink(Sink&&) noexcept = default;
  Sink& operator=(Sink&&) noexcept = default;

  /**
   * @brief Call the implementation of the sink function with the given log
   * record.
   *
   * @param r Log record to pass to the sink function.
   */
  void operator()(LogRecord const& r) { impl_->Call(r); }

 private:
  /**
   * @brief Concept for the type-erased sink implementation. This is an abstract
   * base class that defines the interface for calling the sink function. The
   * SinkModel template will implement this interface for specific sink function
   * types.
   *
   */
  struct SinkConcept {
    virtual void Call(LogRecord const&) = 0;
    virtual ~SinkConcept() = default;
  };

  /**
   * @brief Model for the type-erased sink implementation. This is a concrete
   * implementation of the SinkConcept that wraps a specific sink function type.
   * Stores via shared_ptr to support both movable and non-movable types.
   *
   * @tparam S Type of the sink function, must satisfy the SinkFunction concept.
   */
  template <SinkFunction S>
  struct SinkModel final : SinkConcept {
    explicit SinkModel(S&& s) : s_(std::make_shared<S>(std::forward<S>(s))) {}
    void Call(LogRecord const& r) override { (*s_)(r); }
    std::shared_ptr<S> s_;
  };

  /// Pointer to the type-erased sink implementation.
  std::shared_ptr<SinkConcept> impl_;
};

/**
 * @brief Create a new console sink.
 *
 * @tparam F Formatter function type, must satisfy the FormatterFunction
 * concept. Defaults to @c DefaultFormatter.
 * @return Sink Sink which writes log records to the console using the
 * DefaultFormatter.
 */
template <FormatterFunction F = DefaultFormatter>
[[nodiscard]] inline Sink MakeConsoleSink() noexcept {
  return Sink{[](LogRecord const& r) { std::println("{}", F{}(r)); }};
};

}  // namespace mlogpp
#endif  // MLOGPP_SINK_HPP_
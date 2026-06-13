#pragma once
#ifndef MLOGPP_FILTER_HPP_
#define MLOGPP_FILTER_HPP_
#include "src/level.hpp"

namespace mlogpp {

/**
 * @brief Concept requiring a type to act as a log level filter.
 *
 * A type satisfies @ref LevelFilter if it exposes a @c passes method that
 * accepts a @c LogLevel and returns something convertible to @c bool.
 *
 * @tparam F The type to check.
 */
template <typename F>
concept LevelFilter = requires(F f, LogLevel l) {
  { f.passes(l) } -> std::convertible_to<bool>;
};

/**
 * @brief Runtime-configurable log level filter.
 *
 * Passes any log level at or above @c min_level. The threshold can be changed
 * at runtime, making this suitable for loggers whose verbosity needs to vary
 * without recompilation.
 */
struct DynamicFilter {
  /// Minimum log level that passes the filter.
  LogLevel min_level{LogLevel::kTrace};

  /**
   * @brief Check whether a log level passes the filter.
   *
   * @param l Log level to test.
   * @return @c true if @p l >= @c min_level, @c false otherwise.
   */
  constexpr bool passes(LogLevel const l) const { return l >= min_level; }
};
static_assert(LevelFilter<DynamicFilter>,
              "DynamicFilter must be a LevelFilter");

/**
 * @brief Compile-time log level filter.
 *
 * Passes any log level at or above the template parameter @p L. Because the
 * threshold is part of the type, calls below @p L are dead-code eliminated by
 * the compiler when used with @c if @c constexpr.
 *
 * @tparam L Minimum log level that passes the filter.
 */
template <LogLevel L>
struct StaticFilter {
  /**
   * @brief Check whether a log level passes the filter.
   *
   * @param level Log level to test.
   * @return @c true if @p level >= @p L, @c false otherwise.
   */
  static constexpr bool passes(LogLevel const level) { return level >= L; }
};
static_assert(LevelFilter<StaticFilter<LogLevel::kTrace>>,
              "StaticFilter must be a LevelFilter");

}  // namespace mlogpp

#endif

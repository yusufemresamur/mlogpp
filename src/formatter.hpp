#pragma once
#ifndef MLOGPP_FORMATTER_HPP_
#define MLOGPP_FORMATTER_HPP_

#include "record.hpp"
#include <concepts>
namespace mlogpp {

/**
 * @brief Concept for formatter functions: any callable (const LogRecord&) ->
 * std::string
 *
 * @tparam F Formatter function
 */
template <typename F>
concept FormatterFunction = requires(F f, LogRecord const& r) {
  { f(r) } -> std::convertible_to<std::string>;
};

/**
 * @brief Concept for structured formatters. Must satisfy FormatterFunction and
 * provide Header(), Footer(), and Separator() methods for structured output
 * (e.g., JSON arrays).
 *
 * @tparam F Formatter function type.
 */
template <typename F>
concept StructuredFormatter = FormatterFunction<F> && requires(F f) {
  { f.Header() } -> std::convertible_to<std::string>;
  { f.Footer() } -> std::convertible_to<std::string>;
  { f.Separator() } -> std::convertible_to<std::string>;
};

/**
 * @brief Default formatter for log records. Output format: "[timestamp] [level]
 * [logger_name] [file:line] - message"
 *
 */
struct DefaultFormatter {
  [[nodiscard]] std::string operator()(LogRecord const& r) const {
    return std::format("[{}] [{}] [{}] [{}:{}] - {}",
                       std::chrono::duration_cast<std::chrono::milliseconds>(
                           r.timestamp.time_since_epoch())
                           .count(),
                       ToString(r.level), r.logger_name, r.location.file_name(),
                       r.location.line(), r.message);
  };
};
static_assert(FormatterFunction<DefaultFormatter>,
              "DefaultFormatter must satisfy FormatterFunction concept");

/**
 * @brief JSON formatter for log records. Output format contains keys:
 * timestamp, level, logger_name, file, line, message.
 *
 */
struct JSONFormatter {
  static constexpr std::string Header() { return "["; }
  static constexpr std::string Footer() { return "]"; }
  static constexpr std::string Separator() { return ","; }

  [[nodiscard]] std::string operator()(LogRecord const& r) const {
    return std::format(
        R"({{"timestamp": {}, "level": "{}", "logger_name": "{}", "file": "{}", "line": {}, "message": "{}"}})",
        std::chrono::duration_cast<std::chrono::milliseconds>(
            r.timestamp.time_since_epoch())
            .count(),
        ToString(r.level), r.logger_name, r.location.file_name(),
        r.location.line(), r.message);
  };
};

static_assert(StructuredFormatter<JSONFormatter>,
              "JSONFormatter must satisfy StructuredFormatter concept");

// TODO(yusufemresamur): Fix issue with footer not removed when file is
// reopened.

}  // namespace mlogpp
#endif  // MLOGPP_FORMATTER_HPP_
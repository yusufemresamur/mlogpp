#pragma once
#ifndef MLOGPP_FORMATTER_HPP_
#define MLOGPP_FORMATTER_HPP_

#include "src/record.hpp"
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

static_assert(FormatterFunction<JSONFormatter>,
              "JSONFormatter must satisfy FormatterFunction concept");

}  // namespace mlogpp
#endif  // MLOGPP_FORMATTER_HPP_
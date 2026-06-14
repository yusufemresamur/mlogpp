#pragma once
#ifndef MLOGPP_JSON_FORMATTER_HPP_
#define MLOGPP_JSON_FORMATTER_HPP_
#include "formatter.hpp"
#include "json_escape.hpp"

namespace mlogpp {

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
        ToString(r.level), detail::JsonEscape(r.logger_name),
        detail::JsonEscape(r.location.file_name()), r.location.line(),
        detail::JsonEscape(r.message));
  };
};

static_assert(FormatterFunction<JSONFormatter>,
              "JSONFormatter must satisfy FormatterFunction concept");

}  // namespace mlogpp

#endif
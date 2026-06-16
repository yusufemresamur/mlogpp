#pragma once
#ifndef MLOGPP_RECORD_HPP_
#define MLOGPP_RECORD_HPP_
#include "src/level.hpp"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <format>
#include <functional>
#include <optional>
#include <source_location>
#include <thread>
#include <tuple>

namespace mlogpp {

/**
 * @brief LogRecord struct that holds all information about a log event. This
 * includes the log message, logger name, log level, timestamp, thread ID, and
 * source location.
 *
 */
struct LogRecord {
  // TODO (yusufemresamur) : temporary, find a future proof way to handle
  // timestamps, e.g., allow users to specify their own clock type, etc.
  using ClockType = std::chrono::system_clock;

  /// Name of the logger that emitted the log record.
  std::string logger_name;
  /// Log level of the log record.
  LogLevel level;
  /// Timestamp of when the log record was created.
  ClockType::time_point timestamp;
  /// ID of the thread that emitted the log record.
  std::thread::id thread_id;
  /// Source location of the log record, including file name and line number.
  std::source_location location;

  /**
   * @brief Construct a new Log Record.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param level Log level of the log record.
   * @param fmt Format string for the log message.
   * @param location Source location of the log record.
   * @param timestamp Timestamp of when the log record was created.
   * @param thread_id ID of the thread that emitted the log record.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  LogRecord(LogLevel const level, std::string const fmt,
            std::source_location const location,
            ClockType::time_point const timestamp,
            std::thread::id const thread_id, Args&&... args)
      : level(level),
        timestamp(timestamp),
        thread_id(thread_id),
        location(location),
        format_message_(MakeFormatThunk(fmt, std::forward<Args>(args)...)) {}

  /**
   * @brief Construct a new Log Record.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param level Log level of the log record.
   * @param location Source location of the log record.
   * @param logger_name Name of the logger that emitted the log record.
   * @param fmt Format string for the log message.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  LogRecord(LogLevel const level, std::source_location const location,
            std::string_view const logger_name, std::string_view const fmt,
            Args&&... args)
      : logger_name(logger_name),
        level(level),
        timestamp(ClockType::now()),
        thread_id(std::this_thread::get_id()),
        location(location),
        format_message_(MakeFormatThunk(fmt, std::forward<Args>(args)...)) {}

  /**
   * @brief Return the formatted log message.
   *
   * Formatting is deferred until this is first called (and then memoized), so
   * the std::vformat cost is paid by whoever reads the message — for an async
   * sink that is the background worker, not the calling thread. A sink that
   * never reads the message never pays for formatting. Format errors are caught
   * and rendered as a diagnostic string so a malformed log can never terminate
   * the formatting thread.
   *
   * @return std::string const& The formatted message.
   */
  [[nodiscard]] std::string const& message() const noexcept {
    if (!message_cache_.has_value()) {
      try {
        message_cache_ = format_message_ ? format_message_() : std::string{};
      } catch (std::format_error const& e) {
        message_cache_ = std::format("<format error: {}>", e.what());
      }
    }
    return message_cache_.value();
  }

 private:
  /**
   * @brief Build the type-erased thunk that formats the message on demand.
   *
   * The format string and the arguments are copied (decayed) into the closure
   * so nothing dangles after the logging call returns — std::format_args alone
   * cannot be stored because it only views the original argument objects. The
   * closure is mutable so std::make_format_args receives non-const lvalues.
   */
  template <typename... Args>
  static std::function<std::string()> MakeFormatThunk(
      std::string_view const fmt, Args&&... args) {
    return [fmt = std::string(fmt),
            args = std::make_tuple(
                std::decay_t<Args>(std::forward<Args>(args))...)]() mutable {
      return std::apply(
          [&fmt](auto&... a) {
            return std::vformat(fmt, std::make_format_args(a...));
          },
          args);
    };
  }

  /// Deferred formatter producing the message; invoked lazily by message().
  std::function<std::string()> format_message_;
  /// Memoized result of format_message_; populated on first message() call.
  mutable std::optional<std::string> message_cache_;
};

}  // namespace mlogpp
#endif  // MLOGPP_RECORD_HPP_
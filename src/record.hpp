#pragma once
#ifndef MLOGPP_RECORD_HPP_
#define MLOGPP_RECORD_HPP_
#include "src/level.hpp"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <format>
#include <source_location>
#include <thread>

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

  /// Formatted log message.
  std::string message;
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
   * @param message Format string for the log message.
   * @param location Source location of the log record.
   * @param timestamp Timestamp of when the log record was created.
   * @param thread_id ID of the thread that emitted the log record.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  LogRecord(LogLevel const level, std::string const message,
            std::source_location const location,
            ClockType::time_point const timestamp,
            std::thread::id const thread_id, Args&&... args)
      : message(std::vformat(message, std::make_format_args(args...))),
        level(level),
        timestamp(timestamp),
        thread_id(thread_id),
        location(location) {}

  /**
   * @brief Construct a new Log Record.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param level Log level of the log record.
   * @param location Source location of the log record.
   * @param logger_name Name of the logger that emitted the log record.
   * @param message Format string for the log message.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  LogRecord(LogLevel const level, std::source_location const location,
            std::string_view const logger_name, std::string_view const message,
            Args&&... args)
      : message(std::vformat(message, std::make_format_args(args...))),
        logger_name(logger_name),
        level(level),
        timestamp(ClockType::now()),
        thread_id(std::this_thread::get_id()),
        location(location) {}
};

}  // namespace mlogpp
#endif  // MLOGPP_RECORD_HPP_
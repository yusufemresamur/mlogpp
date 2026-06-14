#pragma once
#ifndef MLOGPP_LOGGER_HPP_
#define MLOGPP_LOGGER_HPP_

#include "src/filter.hpp"
#include "src/fmt_string_with_location.hpp"
#include "src/level.hpp"
#include "src/record.hpp"
#include "src/sink/sink.hpp"
#include <string>
#include <vector>
namespace mlogpp {

/**
 * @brief Logger class that manages log records and dispatches them to sinks.
 * Each logger has a name and a minimum log level. Log records below the minimum
 * level are ignored.
 *
 * @tparam Filter Filter policy
 */
template <LevelFilter Filter = DynamicFilter>
class Logger {
 public:
  /**
   * @brief Construct a Logger instance.
   *
   * @param name Name of the logger, used in log records to identify the source
   * of logs.
   * @param sinks List of sinks to which log records will be dispatched.
   * @param min_level Minimum log level. Log records below this level will be
   * ignored.
   */
  explicit Logger(std::string_view const name,
                  std::vector<Sink> const& sinks = {},
                  LogLevel const min_level = LogLevel::kInfo)
      : name_(name), sinks_(sinks) {
    if constexpr (requires { filter_.min_level = min_level; }) {
      filter_.min_level = min_level;
    }
  };

  /**
   * @brief Add a sink to the logger.
   *
   * @param sink Sink to add.
   * @return Logger& Reference to the logger, allowing for method chaining.
   */
  Logger& AddSink(Sink&& sink) noexcept {
    sinks_.push_back(sink);
    return *this;
  };

  /**
   * @brief Set the minimum log level for the logger. Log records below this
   * level will be ignored.
   *
   * @param level Level to set as the minimum log level.
   * @return Logger& Reference to the logger, allowing for method chaining.
   */
  Logger& SetMinLevel(LogLevel const level) noexcept {
    if constexpr (requires { filter_.min_level = level; }) {
      filter_.min_level = level;
    }
    return *this;
  };

  /**
   * @brief Return the name of the logger.
   *
   * @return std::string_view Name of the logger.
   */
  [[nodiscard]] std::string_view Name() const noexcept { return name_; };

  /**
   * @brief Log a message with the specified log level. If the log level is
   * below the minimum log level of the logger, the message will be ignored.
   *
   * @tparam Level Log level of the message.
   * @tparam Args Types of the arguments for the format string.
   * @param msg Format string with location information.
   * @param args Arguments for the format string.
   */
  template <LogLevel Level, typename... Args>
  void Log(FormatStringWithLocation const msg, Args&&... args) {
    // compile time filter
    if constexpr (!Filter{}.passes(Level)) {
      return;
    }
    // runtime filter
    if (!filter_.passes(Level)) {
      return;
    }
    Emit(LogRecord{Level, msg.loc, Name(), msg.fmt,
                   std::forward<Args>(args)...});
  }

  /**
   * @brief Log a trace message.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param msg Format string with location information.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  void Trace(FormatStringWithLocation const msg, Args&&... args) {
    Log<LogLevel::kTrace>(msg, std::forward<Args>(args)...);
  }

  /**
   * @brief Log a debug message.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param msg Format string with location information.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  void Debug(FormatStringWithLocation const msg, Args&&... args) {
    Log<LogLevel::kDebug>(msg, std::forward<Args>(args)...);
  }

  /**
   * @brief Log an info message.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param msg Format string with location information.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  void Info(FormatStringWithLocation const msg, Args&&... args) {
    Log<LogLevel::kInfo>(msg, std::forward<Args>(args)...);
  }

  /**
   * @brief Log a warning message.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param msg Format string with location information.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  void Warn(FormatStringWithLocation const msg, Args&&... args) {
    Log<LogLevel::kWarn>(msg, std::forward<Args>(args)...);
  }

  /**
   * @brief Log an error message.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param msg Format string with location information.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  void Error(FormatStringWithLocation const msg, Args&&... args) {
    Log<LogLevel::kError>(msg, std::forward<Args>(args)...);
  }

  /**
   * @brief Log a fatal message.
   *
   * @tparam Args Types of the arguments for the format string.
   * @param msg Format string with location information.
   * @param args Arguments for the format string.
   */
  template <typename... Args>
  void Fatal(FormatStringWithLocation const msg, Args&&... args) {
    Log<LogLevel::kFatal>(msg, std::forward<Args>(args)...);
  }

 private:
  /**
   * @brief Emit a log record to all sinks. This is called by the Log method
   * after checking the log level.
   *
   * @param r Log record to emit.
   */
  void Emit(LogRecord const& r) noexcept {
    for (Sink& sink : sinks_) {
      sink(r);
    }
  };

  /// Name of the logger, used in log records to identify the source of logs.
  std::string const name_;
  /// List of sinks to which log records will be dispatched.
  std::vector<Sink> sinks_;

  Filter filter_{};
};

/// Logger with dynamic Level filtering
using DynamicLogger = Logger<DynamicFilter>;

/**
 * @brief Logger with compile time level filtering
 *
 * @tparam Level Minimum level
 */
template <LogLevel Level>
using StaticLogger = Logger<StaticFilter<Level>>;

using LoggerPtr = std::shared_ptr<DynamicLogger>;

}  // namespace mlogpp
#endif  // MLOGPP_LOGGER_HPP_
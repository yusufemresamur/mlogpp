#pragma once
#ifndef MLOGPP_FILE_SINK_HPP_
#define MLOGPP_FILE_SINK_HPP_
#include "sink.hpp"
#include "src/format/formatter.hpp"
#include "src/record.hpp"
#include <filesystem>
#include <fstream>

namespace mlogpp {

/**
 * @brief A sink that writes log records to a file. The file is opened in append
 * mode.
 *
 * @tparam F Formatter function type, must satisfy the FormatterFunction
 * concept. Defaults to @c DefaultFormatter.
 */
template <FormatterFunction F = DefaultFormatter>
class FileSink {
 public:
  /**
   * @brief Construct a File Sink. Throws an exception if the file cannot be
   * opened.
   *
   * Records are buffered and written with a trailing newline but not flushed
   * per line. The stream is flushed only for records at or above @p flush_level
   * and when the sink is destroyed. This trades a small window of crash-time
   * log loss for far fewer write syscalls.
   *
   * @param path Path to the log file. The file will be created if it does not
   * exist.
   * @param flush_level Minimum level that triggers an immediate flush after a
   * record is written. Defaults to @c LogLevel::kError.
   * @throws std::runtime_error If the file cannot be opened.
   */
  explicit FileSink(std::filesystem::path path,
                    LogLevel const flush_level = LogLevel::kError)
      : path_(std::move(path)), flush_level_(flush_level) {
    file_.open(path_, std::ios::app);
    if (!file_) {
      throw std::runtime_error("Failed to open log file: " + path_.string());
    }
  };

  // FileSink manages file stream and is not copyable
  FileSink(FileSink const&) = delete;
  // FileSink manages file stream and is not copyable
  FileSink& operator=(FileSink const&) = delete;

  FileSink(FileSink&&) noexcept = default;
  FileSink& operator=(FileSink&&) noexcept = default;

  ~FileSink() = default;

  /**
   * @brief Write a log record to the file.
   *
   * @param record The log record to write.
   */
  void operator()(LogRecord const& record) { WriteRecord(record); };

 private:
  // Writes the formatted record followed by '\n' but does NOT flush per line:
  // a per-line flush forces a write syscall on every record and dominates the
  // cost of file logging. The OS-buffered stream is flushed only for records at
  // or above flush_level_ (so high-severity logs survive a crash) and on close
  // (the fstream destructor flushes any remaining buffered output).
  void WriteRecord(LogRecord const& r) {
    file_ << formatter_(r) << '\n';
    if (r.level >= flush_level_) {
      file_.flush();
    }
  }

  /// File stream for writing log records to the file.
  std::fstream file_;
  /// Path to the log file.
  std::filesystem::path path_;
  /// Records at or above this level trigger an immediate flush for durability.
  LogLevel flush_level_;
  /// Formatter for formatting log records.
  F formatter_{};
};

/**
 * @brief Make a FileSink with the given path. See @c FileSink for details.
 *
 * @tparam F Formatter function type, must satisfy the FormatterFunction
 * concept. Defaults to @c DefaultFormatter.
 * @param path Path to the log file.
 * @param flush_level Minimum level that triggers an immediate flush after a
 * record is written. Defaults to @c LogLevel::kError.
 * @return Sink FileSink wrapped in a Sink type-erasure wrapper.
 */
template <FormatterFunction F = DefaultFormatter>
[[nodiscard]] inline Sink MakeFileSink(
    std::filesystem::path& path,
    LogLevel const flush_level = LogLevel::kError) {
  return Sink{FileSink<F>{std::move(path), flush_level}};
}

}  // namespace mlogpp

#endif  // MLOGPP_FILE_SINK_HPP_
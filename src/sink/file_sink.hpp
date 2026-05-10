#pragma once
#ifndef MLOGPP_FILE_SINK_HPP_
#define MLOGPP_FILE_SINK_HPP_
#include "src/formatter.hpp"
#include "src/record.hpp"
#include "src/sink/sink.hpp"
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
   * @param path Path to the log file. The file will be created if it does not
   * exist.
   */
  explicit FileSink(std::filesystem::path path) : path_(std::move(path)) {
    file_.open(path_, std::ios::app);
    if (!file_) {
      throw std::runtime_error("Failed to open log file: " + path_.string());
    }
  };

  FileSink(FileSink const&) =
      delete("FileSink manages file stream and is not copyable");
  FileSink& operator=(FileSink const&) =
      delete("FileSink manages file stream and is not copyable");

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
  void WriteRecord(LogRecord const& r) { file_ << formatter_(r) << std::endl; }

  /// File stream for writing log records to the file.
  std::fstream file_;
  /// Path to the log file.
  std::filesystem::path path_;
  /// Formatter for formatting log records.
  F formatter_{};
};

/**
 * @brief Make a FileSink with the given path. See @c FileSink for details.
 *
 * @param path Path to the log file.
 * @param formatter Formatter to use for formatting log records.
 * @return Sink FileSink wrapped in a Sink type-erasure wrapper.
 */
template <FormatterFunction F = DefaultFormatter>
[[nodiscard]] inline Sink MakeFileSink(std::filesystem::path path) {
  return Sink{FileSink<F>{std::move(path)}};
}

}  // namespace mlogpp

#endif  // MLOGPP_FILE_SINK_HPP_
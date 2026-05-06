#pragma once
#ifndef MLOGPP_FILE_SINK_HPP_
#define MLOGPP_FILE_SINK_HPP_
#include "src/formatter.hpp"
#include "src/record.hpp"
#include "src/sink.hpp"
#include <filesystem>
#include <fstream>

namespace mlogpp {

// TODO (yusufemresamur) : Make formatter configurable

/**
 * @brief A sink that writes log records to a file. The file is opened in append
 * mode.
 *
 */
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

  /**
   * @brief Write a log record to the file.
   *
   * @param record The log record to write.
   */
  void operator()(LogRecord const& record) {
    file_ << DefaultFormatter{}(record) << std::endl;
  };

 private:
  /// File stream for writing log records to the file.
  std::fstream file_;
  /// Path to the log file.
  std::filesystem::path path_;
};

/**
 * @brief Make a FileSink with the given path. See @c FileSink for details.
 *
 * @param path Path to the log file.
 * @return Sink FileSink wrapped in a Sink type-erasure wrapper.
 */
[[nodiscard]] inline Sink MakeFileSink(std::filesystem::path path) {
  return Sink{FileSink{std::move(path)}};
}
}  // namespace mlogpp

#endif  // MLOGPP_FILE_SINK_HPP_
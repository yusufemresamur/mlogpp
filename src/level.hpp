#pragma once
#ifndef MLOGPP_LEVEL_HPP_
#define MLOGPP_LEVEL_HPP_

#include <cstdint>
#include <string_view>

namespace mlogpp {

/**
 * @brief Logging level enumeration with six severity levels.
 *
 */
enum class LogLevel : uint8_t {
  kTrace,
  kDebug,
  kInfo,
  kWarn,
  kError,
  kFatal,
};

/**
 * @brief Convert a logging level to its string representation.
 *
 * @param level The logging level to convert.
 * @return constexpr std::string_view  A string_view containing the string
 * representation of the level. Returns "UNKNOWN" for invalid levels.
 */
constexpr std::string_view ToString(LogLevel level) noexcept {
  switch (level) {
    case LogLevel::kTrace:
      return "TRACE";
    case LogLevel::kDebug:
      return "DEBUG";
    case LogLevel::kInfo:
      return "INFO";
    case LogLevel::kWarn:
      return "WARN";
    case LogLevel::kError:
      return "ERROR";
    case LogLevel::kFatal:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}

}  // namespace mlogpp
#endif  // MLOGPP_LEVEL_HPP_
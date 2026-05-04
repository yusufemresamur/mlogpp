#pragma once
#ifndef MLOGPP_FMT_STRING_WITH_LOCATION_HPP_
#define MLOGPP_FMT_STRING_WITH_LOCATION_HPP_

#include <source_location>
#include <string_view>

namespace mlogpp {

template <typename T>
concept StringConvertible = std::convertible_to<T, std::string_view>;

/**
 * @brief A structure that combines a format string with its source location
 * information.
 *
 */
struct FormatStringWithLocation {
  std::string_view const fmt;
  std::source_location const loc;

  /**
   * @brief Construct a FormatStringWithLocation.
   *
   * @tparam T The type of the format string (typically a string literal or
   * std::string_view).
   * @param fmt The format string to be used for logging.
   * @param loc The source location where this constructor is called. Defaults
   * to the call site.
   */
  template <StringConvertible T>
  constexpr FormatStringWithLocation(
      T const& fmt,
      std::source_location const loc = std::source_location::current())
      : fmt(fmt), loc(loc) {}
};
}  // namespace mlogpp
#endif  // MLOGPP_FMT_STRING_WITH_LOCATION_HPP_
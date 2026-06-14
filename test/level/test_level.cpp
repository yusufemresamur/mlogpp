#include "mlogpp/level.hpp"
#include <gtest/gtest.h>
#include <cstdint>

// TODO (yusufemresamur): review AI generated tests

namespace mlogpp {

// ── Underlying type ──────────────────────────────────────────────────────────

TEST(LogLevelTest, UnderlyingTypeIsUint8) {
  static_assert(std::is_same_v<std::underlying_type_t<LogLevel>, uint8_t>);
}

// ── Ordering ─────────────────────────────────────────────────────────────────

TEST(LogLevelTest, SeverityOrderIsAscending) {
  static_assert(LogLevel::kTrace < LogLevel::kDebug);
  static_assert(LogLevel::kDebug < LogLevel::kInfo);
  static_assert(LogLevel::kInfo < LogLevel::kWarn);
  static_assert(LogLevel::kWarn < LogLevel::kError);
  static_assert(LogLevel::kError < LogLevel::kFatal);
}

// ── ToString — all named levels
// ───────────────────────────────────────────────

TEST(ToStringTest, NamedLevelsReturnExpectedStrings) {
  static_assert(ToString(LogLevel::kTrace) == "TRACE");
  static_assert(ToString(LogLevel::kDebug) == "DEBUG");
  static_assert(ToString(LogLevel::kInfo) == "INFO");
  static_assert(ToString(LogLevel::kWarn) == "WARN");
  static_assert(ToString(LogLevel::kError) == "ERROR");
  static_assert(ToString(LogLevel::kFatal) == "FATAL");
}

// ── ToString — invalid / out-of-range value
// ───────────────────────────────────

TEST(ToStringTest, OutOfRangeValueReturnsUnknown) {
  // Cast an integer that doesn't correspond to any enumerator.
  auto const invalid = static_cast<LogLevel>(0xFF);
  EXPECT_EQ(ToString(invalid), "UNKNOWN");
}

TEST(ToStringTest, ValueJustPastFatalReturnsUnknown) {
  constexpr auto kPastFatal =
      static_cast<LogLevel>(static_cast<uint8_t>(LogLevel::kFatal) + 1);
  EXPECT_EQ(ToString(kPastFatal), "UNKNOWN");
}

// ── ToString — return type
// ────────────────────────────────────────────────────

TEST(ToStringTest, ReturnTypeIsStringView) {
  static_assert(
      std::is_same_v<decltype(ToString(LogLevel::kInfo)), std::string_view>);
}

TEST(ToStringTest, IsNoexcept) {
  static_assert(noexcept(ToString(LogLevel::kInfo)));
}

TEST(ToStringTest, IsConstexpr) {
  constexpr std::string_view kSv = ToString(LogLevel::kWarn);
  static_assert(kSv == "WARN");
}

// ── ToString — returned strings are non-empty
// ─────────────────────────────────

TEST(ToStringTest, AllNamedLevelsReturnNonEmptyString) {
  constexpr LogLevel kLevels[] = {
      LogLevel::kTrace, LogLevel::kDebug, LogLevel::kInfo,
      LogLevel::kWarn,  LogLevel::kError, LogLevel::kFatal,
  };
  for (auto const level : kLevels) {
    EXPECT_FALSE(ToString(level).empty())
        << "Empty string for level " << static_cast<int>(level);
  }
}

TEST(ToStringTest, AllNamedLevelsReturnDistinctStrings) {
  constexpr std::string_view kExpected[] = {
      "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
  };
  constexpr LogLevel kLevels[] = {
      LogLevel::kTrace, LogLevel::kDebug, LogLevel::kInfo,
      LogLevel::kWarn,  LogLevel::kError, LogLevel::kFatal,
  };
  for (std::size_t i = 0; i < std::size(kLevels); ++i) {
    EXPECT_EQ(ToString(kLevels[i]), kExpected[i]);
    for (std::size_t j = i + 1; j < std::size(kLevels); ++j) {
      EXPECT_NE(ToString(kLevels[i]), ToString(kLevels[j]))
          << "Levels " << i << " and " << j << " have the same string";
    }
  }
}

}  // namespace mlogpp

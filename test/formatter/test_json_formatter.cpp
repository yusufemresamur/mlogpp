#include "mlogpp/format/json_formatter.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <source_location>

// TODO (yusufemresamur): review AI generated tests

namespace {

[[nodiscard]] mlogpp::LogRecord MakeRecord(
    mlogpp::LogLevel level, std::string_view message,
    mlogpp::LogRecord::ClockType::time_point timestamp,
    std::source_location location, std::string_view logger_name = "") {
  mlogpp::LogRecord rec(level, std::string(message), location, timestamp,
                        std::this_thread::get_id());
  rec.logger_name = logger_name;
  return rec;
}

constexpr mlogpp::LogRecord::ClockType::time_point kEpoch{};
constexpr mlogpp::LogRecord::ClockType::time_point kFixedTime{
    std::chrono::milliseconds(1000)};

}  // namespace

TEST(JSONFormatterTest, SatisfiesFormatterFunctionConcept) {
  static_assert(mlogpp::FormatterFunction<mlogpp::JSONFormatter>);
}

TEST(JSONFormatterTest, FormatAtEpoch) {
  auto const loc = std::source_location::current();
  auto const rec =
      MakeRecord(mlogpp::LogLevel::kInfo, "hello", kEpoch, loc, "svc");

  auto const result = mlogpp::JSONFormatter{}(rec);

  auto const expected = std::format(
      R"({{"timestamp": 0, "level": "INFO", "logger_name": "svc", "file": "{}", "line": {}, "message": "hello"}})",
      loc.file_name(), loc.line());
  EXPECT_EQ(result, expected);
}

TEST(JSONFormatterTest, FormatAtFixedTimestamp) {
  auto const loc = std::source_location::current();
  auto const rec =
      MakeRecord(mlogpp::LogLevel::kWarn, "watch out", kFixedTime, loc, "app");

  auto const result = mlogpp::JSONFormatter{}(rec);

  auto const expected = std::format(
      R"({{"timestamp": 1000, "level": "WARN", "logger_name": "app", "file": "{}", "line": {}, "message": "watch out"}})",
      loc.file_name(), loc.line());
  EXPECT_EQ(result, expected);
}

TEST(JSONFormatterTest, AllLogLevels) {
  constexpr std::pair<mlogpp::LogLevel, std::string_view> kCases[] = {
      {mlogpp::LogLevel::kTrace, "TRACE"}, {mlogpp::LogLevel::kDebug, "DEBUG"},
      {mlogpp::LogLevel::kInfo, "INFO"},   {mlogpp::LogLevel::kWarn, "WARN"},
      {mlogpp::LogLevel::kError, "ERROR"}, {mlogpp::LogLevel::kFatal, "FATAL"},
  };

  auto const loc = std::source_location::current();
  mlogpp::JSONFormatter const fmt;
  for (auto const& [level, expected_level_str] : kCases) {
    auto const rec = MakeRecord(level, "m", kEpoch, loc, "l");
    auto const result = fmt(rec);
    auto const level_kv = std::format(R"("level": "{}")", expected_level_str);
    EXPECT_NE(result.find(level_kv), std::string::npos)
        << "Missing level key-value for " << expected_level_str;
  }
}

TEST(JSONFormatterTest, OutputIsValidJsonStructure) {
  auto const loc = std::source_location::current();
  auto const rec =
      MakeRecord(mlogpp::LogLevel::kError, "oops", kFixedTime, loc, "svc");

  auto const result = mlogpp::JSONFormatter{}(rec);

  EXPECT_EQ(result.front(), '{');
  EXPECT_EQ(result.back(), '}');
  EXPECT_NE(result.find("\"timestamp\":"), std::string::npos);
  EXPECT_NE(result.find("\"level\":"), std::string::npos);
  EXPECT_NE(result.find("\"logger_name\":"), std::string::npos);
  EXPECT_NE(result.find("\"file\":"), std::string::npos);
  EXPECT_NE(result.find("\"line\":"), std::string::npos);
  EXPECT_NE(result.find("\"message\":"), std::string::npos);
}

TEST(JSONFormatterTest, MessageValuePresent) {
  auto const loc = std::source_location::current();
  auto const rec = MakeRecord(mlogpp::LogLevel::kFatal, "critical failure",
                              kEpoch, loc, "core");

  auto const result = mlogpp::JSONFormatter{}(rec);

  EXPECT_NE(result.find("\"message\": \"critical failure\""),
            std::string::npos);
}

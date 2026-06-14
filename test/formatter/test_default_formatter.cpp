#include "mlogpp/format/formatter.hpp"
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

TEST(DefaultFormatterTest, SatisfiesFormatterFunctionConcept) {
  static_assert(mlogpp::FormatterFunction<mlogpp::DefaultFormatter>);
}

TEST(DefaultFormatterTest, FormatAtEpoch) {
  auto const loc = std::source_location::current();
  auto const rec =
      MakeRecord(mlogpp::LogLevel::kInfo, "hello", kEpoch, loc, "test");

  auto const result = mlogpp::DefaultFormatter{}(rec);

  EXPECT_EQ(result, std::format("[0] [INFO] [test] [{}:{}] - hello",
                                loc.file_name(), loc.line()));
}

TEST(DefaultFormatterTest, FormatAtFixedTimestamp) {
  auto const loc = std::source_location::current();
  auto const rec =
      MakeRecord(mlogpp::LogLevel::kDebug, "msg", kFixedTime, loc, "logger");

  auto const result = mlogpp::DefaultFormatter{}(rec);

  EXPECT_EQ(result, std::format("[1000] [DEBUG] [logger] [{}:{}] - msg",
                                loc.file_name(), loc.line()));
}

TEST(DefaultFormatterTest, AllLogLevels) {
  constexpr std::pair<mlogpp::LogLevel, std::string_view> kCases[] = {
      {mlogpp::LogLevel::kTrace, "TRACE"}, {mlogpp::LogLevel::kDebug, "DEBUG"},
      {mlogpp::LogLevel::kInfo, "INFO"},   {mlogpp::LogLevel::kWarn, "WARN"},
      {mlogpp::LogLevel::kError, "ERROR"}, {mlogpp::LogLevel::kFatal, "FATAL"},
  };

  auto const loc = std::source_location::current();
  mlogpp::DefaultFormatter const fmt;
  for (auto const& [level, expected_level_str] : kCases) {
    auto const rec = MakeRecord(level, "m", kEpoch, loc, "l");
    auto const result = fmt(rec);
    EXPECT_NE(result.find(expected_level_str), std::string::npos)
        << "Missing level string for " << expected_level_str;
  }
}

TEST(DefaultFormatterTest, MessageAppearsAfterDash) {
  auto const loc = std::source_location::current();
  auto const rec = MakeRecord(mlogpp::LogLevel::kError, "something went wrong",
                              kFixedTime, loc, "svc");

  auto const result = mlogpp::DefaultFormatter{}(rec);

  EXPECT_NE(result.find("- something went wrong"), std::string::npos);
}

TEST(DefaultFormatterTest, FileAndLinePresent) {
  auto const loc = std::source_location::current();
  auto const rec = MakeRecord(mlogpp::LogLevel::kInfo, "m", kEpoch, loc, "l");

  auto const result = mlogpp::DefaultFormatter{}(rec);

  EXPECT_NE(result.find(loc.file_name()), std::string::npos);
  EXPECT_NE(result.find(std::to_string(loc.line())), std::string::npos);
}
